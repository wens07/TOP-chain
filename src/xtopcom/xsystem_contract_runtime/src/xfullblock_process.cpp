#include "xsystem_contract_runtime/xsystem_contract_manager.h"
#include "xsystem_contract_runtime/xerror/xerror.h"

#include "xdata/xfull_tableblock.h"
#include "xdata/xslash.h"
#include "xstake/xstake_algorithm.h"
#include "xvledger/xvcnode.h"
#include "xvledger/xvledger.h"

NS_BEG2(top, contract_runtime)
using namespace top::data;
using namespace top::xstake;

xunqualified_node_info_t process_slash_statistic_data(top::data::xstatistics_data_t const& block_statistic_data, base::xvnodesrv_t * node_service) {
    xunqualified_node_info_t res_node_info;

    // process one full tableblock statistic data
    for (auto const & static_item: block_statistic_data.detail) {
        auto elect_statistic = static_item.second;
        for (auto const & group_item: elect_statistic.group_statistics_data) {
            xgroup_related_statistics_data_t const& group_account_data = group_item.second;
            common::xgroup_address_t const& group_addr = group_item.first;
            xvip2_t const& group_xvip2 = top::common::xip2_t{
                group_addr.network_id(),
                group_addr.zone_id(),
                group_addr.cluster_id(),
                group_addr.group_id(),
                (uint16_t)group_account_data.account_statistics_data.size(),
                static_item.first
            };
            // process auditor group
            if (top::common::has<top::common::xnode_type_t::auditor>(group_addr.type())) {
                for (std::size_t slotid = 0; slotid < group_account_data.account_statistics_data.size(); ++slotid) {
                    auto account_addr = node_service->get_group(group_xvip2)->get_node(slotid)->get_account();
                    res_node_info.auditor_info[common::xnode_id_t{account_addr}].subset_count += group_account_data.account_statistics_data[slotid].vote_data.block_count;
                    res_node_info.auditor_info[common::xnode_id_t{account_addr}].block_count += group_account_data.account_statistics_data[slotid].vote_data.vote_count;
                    xdbg("[xtable_statistic_info_collection_contract][do_unqualified_node_slash] incremental auditor data: {gourp id: %d, account addr: %s, slot id: %u, subset count: %u, block_count: %u}", group_addr.group_id().value(), account_addr.c_str(),
                        slotid, group_account_data.account_statistics_data[slotid].vote_data.block_count, group_account_data.account_statistics_data[slotid].vote_data.vote_count);
                }
            } else if (top::common::has<top::common::xnode_type_t::validator>(group_addr.type())) {// process validator group
                for (std::size_t slotid = 0; slotid < group_account_data.account_statistics_data.size(); ++slotid) {
                    auto account_addr = node_service->get_group(group_xvip2)->get_node(slotid)->get_account();
                    res_node_info.validator_info[common::xnode_id_t{account_addr}].subset_count += group_account_data.account_statistics_data[slotid].vote_data.block_count;
                    res_node_info.validator_info[common::xnode_id_t{account_addr}].block_count += group_account_data.account_statistics_data[slotid].vote_data.vote_count;
                    xdbg("[xtable_statistic_info_collection_contract][do_unqualified_node_slash] incremental validator data: {gourp id: %d, account addr: %s, slot id: %u, subset count: %u, block_count: %u}", group_addr.group_id().value(), account_addr.c_str(),
                        slotid, group_account_data.account_statistics_data[slotid].vote_data.block_count, group_account_data.account_statistics_data[slotid].vote_data.vote_count);
                }

            } else { // invalid group
                xwarn("[xtable_statistic_info_collection_contract][do_unqualified_node_slash] invalid group id: %d", group_addr.group_id().value());
                std::error_code ec{ contract_runtime::system::error::xerrc_t::unknown_error };
                top::error::throw_error(ec, "[xtable_statistic_info_collection_contract][do_unqualified_node_slash] invalid group");
            }

        }

    }

    return res_node_info;
}


std::map<common::xgroup_address_t, xstake::xgroup_workload_t> process_workload_statistic_data(xstatistics_data_t const & statistic_data, base::xvnodesrv_t * node_service) {
    std::map<common::xgroup_address_t, xgroup_workload_t> group_workload;
    auto workload_per_tableblock = XGET_ONCHAIN_GOVERNANCE_PARAMETER(workload_per_tableblock);
    auto workload_per_tx = XGET_ONCHAIN_GOVERNANCE_PARAMETER(workload_per_tx);
    for (auto const & static_item : statistic_data.detail) {
        auto elect_statistic = static_item.second;
        for (auto const & group_item : elect_statistic.group_statistics_data) {
            common::xgroup_address_t const & group_addr = group_item.first;
            xgroup_related_statistics_data_t const & group_account_data = group_item.second;
            xvip2_t const & group_xvip2 = top::common::xip2_t{group_addr.network_id(),
                                                              group_addr.zone_id(),
                                                              group_addr.cluster_id(),
                                                              group_addr.group_id(),
                                                              (uint16_t)group_account_data.account_statistics_data.size(),
                                                              static_item.first};
            for (size_t slotid = 0; slotid < group_account_data.account_statistics_data.size(); ++slotid) {
                auto account_str = node_service->get_group(group_xvip2)->get_node(slotid)->get_account();
                uint32_t block_count = group_account_data.account_statistics_data[slotid].block_data.block_count;
                uint32_t tx_count = group_account_data.account_statistics_data[slotid].block_data.transaction_count;
                uint32_t workload = block_count * workload_per_tableblock + tx_count * workload_per_tx;
                if (workload > 0) {
                    auto it2 = group_workload.find(group_addr);
                    if (it2 == group_workload.end()) {
                        xgroup_workload_t group_workload_info;
                        auto ret = group_workload.insert(std::make_pair(group_addr, group_workload_info));
                        it2 = ret.first;
                    }

                    it2->second.m_leader_count[account_str] += workload;
                }
            }
        }
    }
    return group_workload;
}

void  process_fulltable(xblock_ptr_t const& block) {
    auto block_owner = block->get_block_owner();
    auto block_height = block->get_height();
    xdbg("process_fulltable fullblock process, owner: %s, height: %" PRIu64, block->get_block_owner().c_str(), block_height);
    base::xauto_ptr<base::xvblock_t> full_block = base::xvchain_t::instance().get_xblockstore()->load_block_object(base::xvaccount_t{block_owner}, block_height, base::enum_xvblock_flag_committed, true);

    xfull_tableblock_t* full_tableblock = dynamic_cast<xfull_tableblock_t*>(full_block.get());
    auto fulltable_statisitc_data = full_tableblock->get_table_statistics();
    base::xvnodesrv_t * node_service_ptr;
    auto slash_info = process_slash_statistic_data(fulltable_statisitc_data, node_service_ptr);
    auto workload_info = process_workload_statistic_data(fulltable_statisitc_data, node_service_ptr);


    base::xstream_t stream(base::xcontext_t::instance());
    slash_info.serialize_to(stream);
    MAP_OBJECT_SERIALIZE2(stream, workload_info);
    stream << block_height;
    stream << block->get_pledge_balance_change_tgas();
    std::string action_params = std::string((char *)stream.data(), stream.size());


    // XMETRICS_GAUGE(metrics::xmetircs_tag_t::contract_table_fullblock_event, 1);
    // on_fulltableblock_event("table_address", "on_collect_statistic_info", action_params, block->get_timestamp(), (uint16_t)table_id);
}

NS_END2