﻿// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xblockmaker/xunit_builder.h"

#include "xblockmaker/xblockmaker_error.h"
#include "xchain_upgrade/xchain_upgrade_center.h"
#include "xcontract_vm/xaccount_vm.h"
#include "xdata/xblockbuild.h"
#include "xdata/xblocktool.h"
#include "xdata/xemptyblock.h"
#include "xdata/xfullunit.h"
#include "xstore/xaccount_context.h"
#include "xsystem_contract_runtime/xsystem_contract_manager.h"
#include "xtxexecutor/xtransaction_executor.h"
#include "xvledger/xreceiptid.h"
#include "xvledger/xvledger.h"
#include "xvledger/xvstatestore.h"

#include <cinttypes>
#include <string>

NS_BEG2(top, blockmaker)

xlightunit_builder_t::xlightunit_builder_t() {

}

void xlightunit_builder_t::alloc_tx_receiptid(const std::vector<xcons_transaction_ptr_t> & input_txs, const base::xreceiptid_state_ptr_t & receiptid_state) {
    for (auto & tx : input_txs) {
        data::xblocktool_t::alloc_transaction_receiptid(tx, receiptid_state);
    }
}

xblock_ptr_t xlightunit_builder_t::create_block(const xblock_ptr_t & prev_block, const data::xblock_consensus_para_t & cs_para, const xlightunit_block_para_t & lightunit_para, const base::xreceiptid_state_ptr_t & receiptid_state) {
    alloc_tx_receiptid(lightunit_para.get_input_txs(), receiptid_state);

    xlightunit_build_t bbuild(prev_block.get(), lightunit_para, cs_para);
    base::xvblock_t* _proposal_block = data::xblocktool_t::create_next_lightunit(lightunit_para, prev_block.get(), cs_para);
    xblock_ptr_t proposal_unit;
    proposal_unit.attach((data::xblock_t*)_proposal_block);
    return proposal_unit;
}

xblock_ptr_t        xlightunit_builder_t::build_block(const xblock_ptr_t & prev_block,
                                                      const xobject_ptr_t<base::xvbstate_t> & prev_bstate,
                                                      const data::xblock_consensus_para_t & cs_para,
                                                      xblock_builder_para_ptr_t & build_para) {
    XMETRICS_TIMER(metrics::cons_unitbuilder_lightunit_tick);
    const std::string & account = prev_block->get_account();
    std::shared_ptr<xlightunit_builder_para_t> lightunit_build_para = std::dynamic_pointer_cast<xlightunit_builder_para_t>(build_para);
    xassert(lightunit_build_para != nullptr);

    const std::vector<xcons_transaction_ptr_t> & input_txs = lightunit_build_para->get_origin_txs();
    xassert(!input_txs.empty());

    bool has_run_contract_tx{false};
    bool has_other_tx{false};

    auto const & chain_config = chain_upgrade::xchain_fork_config_center_t::chain_fork_config();
    if (chain_upgrade::xchain_fork_config_center_t::is_forked(chain_config.new_system_contract_runtime_fork_point, cs_para.get_timestamp())) {
        for (auto const & tx : input_txs) {
            if (tx->get_tx_type() == enum_xtransaction_type::xtransaction_type_run_contract) {
                has_run_contract_tx = true;
            } else {
                has_other_tx = true;
            }
        }
    }

    if (has_run_contract_tx && has_other_tx) {
        xerror("[xlightunit_builder_t::build_block] has run_contract_tx and other_tx same time!");
        return nullptr;
    }

    if (has_run_contract_tx) {
        for (auto const & tx : input_txs) {
            xdbg("------>new vm, %s, %s, %d", tx->get_source_addr().c_str(), tx->get_target_addr().c_str(), tx->get_tx_subtype());
        }

        xassert(!cs_para.get_table_account().empty());
        xassert(!cs_para.get_random_seed().empty());
        xassert(cs_para.get_table_proposal_height() > 0);

        auto * system_contract_manager = contract_runtime::system::xsystem_contract_manager_t::instance();
        xassert(system_contract_manager != nullptr);
        contract_vm::xaccount_vm_t vm(make_observer(system_contract_manager));

        auto tx_address = common::xaccount_address_t{prev_block->get_account()};
        auto _temp_header = base::xvblockbuild_t::build_proposal_header(prev_block.get(), cs_para.get_clock());
        auto proposal_bstate = make_object_ptr<base::xvbstate_t>(*_temp_header.get(), *prev_bstate.get());
        auto result = vm.execute(input_txs, make_observer(proposal_bstate.get()), cs_para);

        lightunit_build_para->set_fail_txs(result.failed_tx_assemble);
        lightunit_build_para->set_pack_txs(result.success_tx_assemble);
        if (result.success_tx_assemble.size() == 0) {
            build_para->set_error_code(xblockmaker_error_tx_execute);
            return nullptr;
        }

        xlightunit_block_para_t lightunit_para;
        lightunit_para.set_input_txs(result.success_tx_assemble);
        lightunit_para.set_fullstate_bin(result.bincode);
        lightunit_para.set_binlog(result.binlog);

        base::xreceiptid_state_ptr_t receiptid_state = lightunit_build_para->get_receiptid_state();
        alloc_tx_receiptid(result.success_tx_assemble, receiptid_state);
        base::xvblock_t* _proposal_block = data::xblocktool_t::create_next_lightunit(lightunit_para, prev_block.get(), cs_para);
        xblock_ptr_t proposal_unit;
        proposal_unit.attach((data::xblock_t *)_proposal_block);
        return proposal_unit;
    } else {
        for (auto const & tx : input_txs) {
            xdbg("------>old vm, %s, %s, %d", tx->get_source_addr().c_str(), tx->get_target_addr().c_str(), tx->get_tx_subtype());
        }

        txexecutor::xbatch_txs_result_t exec_result;
        int exec_ret = txexecutor::xtransaction_executor::exec_batch_txs(prev_block.get(), prev_bstate, cs_para, input_txs, exec_result);
        xinfo(
            "xlightunit_builder_t::build_block "
            "%s,account=%s,height=%ld,exec_ret=%d,succtxs_count=%zu,failtxs_count=%zu,unconfirm_count=%d,binlog_size=%zu,binlog=%ld,state_size=%zu",
            cs_para.dump().c_str(),
            prev_block->get_account().c_str(),
            prev_block->get_height() + 1,
            exec_ret,
            exec_result.m_exec_succ_txs.size(),
            exec_result.m_exec_fail_txs.size(),
            exec_result.m_unconfirm_tx_num,
            exec_result.m_property_binlog.size(),
            base::xhash64_t::digest(exec_result.m_property_binlog),
            exec_result.m_full_state.size());
        // some send txs may execute fail but some recv/confirm txs may execute successfully
        if (!exec_result.m_exec_fail_txs.empty()) {
            lightunit_build_para->set_fail_txs(exec_result.m_exec_fail_txs);
        }
        if (exec_ret != xsuccess) {
            build_para->set_error_code(xblockmaker_error_tx_execute);
            return nullptr;
        }

        lightunit_build_para->set_tgas_balance_change(exec_result.m_tgas_balance_change);
        lightunit_build_para->set_pack_txs(exec_result.m_exec_succ_txs);

        xlightunit_block_para_t lightunit_para;
        // set lightunit para by tx result
        lightunit_para.set_input_txs(exec_result.m_exec_succ_txs);
        lightunit_para.set_account_unconfirm_sendtx_num(exec_result.m_unconfirm_tx_num);
        lightunit_para.set_fullstate_bin(exec_result.m_full_state);
        lightunit_para.set_binlog(exec_result.m_property_binlog);
        return create_block(prev_block, cs_para, lightunit_para, lightunit_build_para->get_receiptid_state());
    }
}

std::string     xfullunit_builder_t::make_binlog(const base::xauto_ptr<base::xvheader_t> & _temp_header,
                                                const xobject_ptr_t<base::xvbstate_t> & prev_bstate) {
    xobject_ptr_t<base::xvbstate_t> proposal_bstate = make_object_ptr<base::xvbstate_t>(*_temp_header.get(), *prev_bstate.get());

    std::string property_snapshot;
    auto canvas = proposal_bstate->rebase_change_to_snapshot();
    canvas->encode(property_snapshot);
    xassert(!property_snapshot.empty());
    return property_snapshot;
}


xblock_ptr_t        xfullunit_builder_t::build_block(const xblock_ptr_t & prev_block,
                                                    const xobject_ptr_t<base::xvbstate_t> & prev_bstate,
                                                    const data::xblock_consensus_para_t & cs_para,
                                                    xblock_builder_para_ptr_t & build_para) {
    XMETRICS_TIMER(metrics::cons_unitbuilder_fullunit_tick);

    base::xauto_ptr<base::xvheader_t> _temp_header = base::xvblockbuild_t::build_proposal_header(prev_block.get(), cs_para.get_clock());
    xfullunit_block_para_t para;
    para.m_property_snapshot = make_binlog(_temp_header, prev_bstate);
    para.m_first_unit_height = prev_bstate->get_last_fullblock_height();
    para.m_first_unit_hash = prev_bstate->get_last_fullblock_hash();
    xinfo("xfullunit_builder_t::build_block %s,account=%s,height=%ld,binlog_size=%zu,binlog=%ld",
        cs_para.dump().c_str(), prev_block->get_account().c_str(), prev_block->get_height() + 1,
        para.m_property_snapshot.size(), base::xhash64_t::digest(para.m_property_snapshot));

    base::xvblock_t* _proposal_block = data::xblocktool_t::create_next_fullunit(para, prev_block.get(), cs_para);
    xblock_ptr_t proposal_unit;
    proposal_unit.attach((data::xblock_t*)_proposal_block);
    return proposal_unit;
}

xblock_ptr_t        xemptyunit_builder_t::build_block(const xblock_ptr_t & prev_block,
                                                    const xobject_ptr_t<base::xvbstate_t> & prev_bstate,
                                                    const data::xblock_consensus_para_t & cs_para,
                                                    xblock_builder_para_ptr_t & build_para) {
    base::xvblock_t* _proposal_block = data::xblocktool_t::create_next_emptyblock(prev_block.get(), cs_para);
    xblock_ptr_t proposal_unit;
    proposal_unit.attach((data::xblock_t*)_proposal_block);
    return proposal_unit;
}

NS_END2
