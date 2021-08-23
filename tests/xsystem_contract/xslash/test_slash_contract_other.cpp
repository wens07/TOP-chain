// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>
#include <string>

#include "xbase/xobject_ptr.h"
#include "xbasic/xasio_io_context_wrapper.h"
#include "xbasic/xtimer_driver.h"
#include "xblockstore/xblockstore_face.h"
#include "xchain_timer/xchain_timer.h"
#include "xconfig/xpredefined_configurations.h"
#include "xdata/xtransaction.h"
#include "xdata/xnative_contract_address.h"
#include "xdata/xblocktool.h"
#include "xloader/xconfig_onchain_loader.h"
#include "xstake/xstake_algorithm.h"
#include "xstore/xstore_face.h"

#include "xchain_upgrade/xchain_data_galileo.h"
#include "tests/mock/xdatamock_unit.hpp"
#include "nlohmann/json.hpp"

#define private public
#include "xvm/xsystem_contracts/xslash/xtable_statistic_info_collection_contract.h"
#include "xvm/xsystem_contracts/xslash/xzec_slash_info_contract.h"
#include "xvm/xvm_service.h"
#include "xvm/xvm_trace.h"
#include "xdata/xgenesis_data.h"
#include "xvm/manager/xcontract_manager.h"
#include "xvm/xsystem_contracts/deploy/xcontract_deploy.h"


using namespace top;
using namespace top::xvm;
using namespace top::contract;
using namespace top::xvm::xcontract;
using json = nlohmann::json;

std::string shard_table_slash_addr = std::string(sys_contract_sharding_statistic_info_addr) + std::string("@3");

class test_slash_contract_other: public xzec_slash_info_contract, public testing::Test {
public:
    test_slash_contract_other(): xzec_slash_info_contract{common::xnetwork_id_t{0}}{};

    void SetUp() {
        m_store = xstore_factory::create_store_with_memdb();
        top::base::xvchain_t::instance().set_xdbstore(m_store.get());
        m_blockstore.attach(top::store::get_vblockstore());
        auto mbus =  top::make_unique<mbus::xmessage_bus_t>(true, 1000);
        std::shared_ptr<top::xbase_io_context_wrapper_t> io_object = std::make_shared<top::xbase_io_context_wrapper_t>();
        std::shared_ptr<top::xbase_timer_driver_t> timer_driver = std::make_shared<top::xbase_timer_driver_t>(io_object);
        auto chain_timer = top::make_object_ptr<time::xchain_timer_t>(timer_driver);
        auto& config_center = top::config::xconfig_register_t::get_instance();

        config::xconfig_loader_ptr_t loader = std::make_shared<loader::xconfig_onchain_loader_t>(make_observer(m_store), make_observer(mbus.get()), make_observer(chain_timer));
        config_center.add_loader(loader);
        config_center.load();

        // table slash statistic contract
        xcontract_manager_t::instance().register_contract<xtable_statistic_info_collection_contract>(common::xaccount_address_t{sys_contract_sharding_statistic_info_addr}, common::xtopchain_network_id);
        xcontract_manager_t::instance().register_contract_cluster_address(common::xaccount_address_t{sys_contract_sharding_statistic_info_addr}, common::xaccount_address_t{shard_table_slash_addr});
        // xcontract_manager_t::instance().setup_chain(common::xaccount_address_t{sys_contract_sharding_statistic_info_addr}, m_blockstore.get());


        xcontract_manager_t::instance().register_contract<xzec_slash_info_contract>(common::xaccount_address_t{sys_contract_zec_slash_info_addr}, common::xtopchain_network_id);
        xcontract_manager_t::instance().register_contract_cluster_address(common::xaccount_address_t{sys_contract_zec_slash_info_addr}, common::xaccount_address_t{sys_contract_zec_slash_info_addr});
        xcontract_manager_t::instance().setup_chain(common::xaccount_address_t{sys_contract_zec_slash_info_addr}, m_blockstore.get());
    }

    void TearDown(){}

    static data::xtransaction_ptr_t on_collect_statistic_info(uint64_t timestamp) {
        xaction_t source_action;
        xaction_t destination_action;
        source_action.set_account_addr(shard_table_slash_addr);
        destination_action.set_account_addr(shard_table_slash_addr);
        destination_action.set_action_name("on_collect_statistic_info");

        top::base::xstream_t target_stream(base::xcontext_t::instance());
        target_stream << timestamp;
        destination_action.set_action_param(std::string((char*) target_stream.data(), target_stream.size()));

        data::xtransaction_ptr_t slash_colletion_trx = make_object_ptr<xtransaction_t>();
        slash_colletion_trx->set_source_action(source_action);
        slash_colletion_trx->set_target_action(destination_action);
        return slash_colletion_trx;
    }

    static data::xtransaction_ptr_t summarize_slash_info(std::string const& slash_info) {

        xaction_t source_action;
        xaction_t destination_action;
        // source_action.set_account_addr(sys_contract_zec_slash_info_addr);
        source_action.set_account_addr(shard_table_slash_addr);
        destination_action.set_account_addr(sys_contract_zec_slash_info_addr);
        destination_action.set_action_name("summarize_slash_info");


        destination_action.set_action_param(slash_info);

        data::xtransaction_ptr_t slash_summarize_trx = make_object_ptr<xtransaction_t>();
        slash_summarize_trx->set_source_action(source_action);
        slash_summarize_trx->set_target_action(destination_action);
        return slash_summarize_trx;

    }

    static data::xtransaction_ptr_t do_unqualified_node_slash(uint64_t timestamp) {
        xaction_t source_action;
        xaction_t destination_action;
        source_action.set_account_addr(sys_contract_zec_slash_info_addr);
        destination_action.set_account_addr(sys_contract_zec_slash_info_addr);
        destination_action.set_action_name("do_unqualified_node_slash");

        top::base::xstream_t target_stream(base::xcontext_t::instance());
        target_stream << timestamp;
        destination_action.set_action_param(std::string((char*) target_stream.data(), target_stream.size()));

        data::xtransaction_ptr_t slash_colletion_trx = make_object_ptr<xtransaction_t>();
        slash_colletion_trx->set_source_action(source_action);
        slash_colletion_trx->set_target_action(destination_action);
        return slash_colletion_trx;
    }

   static xobject_ptr_t<xstore_face_t> m_store;
   static xobject_ptr_t<base::xvblockstore_t> m_blockstore;
   static shared_ptr<xaccount_context_t> m_table_slash_account_ctx_ptr;
   static shared_ptr<xaccount_context_t> m_zec_slash_account_ctx_ptr;

};

xobject_ptr_t<xstore_face_t> test_slash_contract_other::m_store;
xobject_ptr_t<base::xvblockstore_t> test_slash_contract_other::m_blockstore;
shared_ptr<xaccount_context_t> test_slash_contract_other::m_table_slash_account_ctx_ptr;
shared_ptr<xaccount_context_t> test_slash_contract_other::m_zec_slash_account_ctx_ptr;


TEST_F(test_slash_contract_other, zec_setup_reset_data) {
    using namespace top::mock;
    xdatamock_unit  zec_account{sys_contract_zec_slash_info_addr};
    m_zec_slash_account_ctx_ptr = make_shared<xaccount_context_t>(zec_account.get_account_state(), m_store.get());
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPORPERTY_CONTRACT_UNQUALIFIED_NODE_KEY);
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPROPERTY_CONTRACT_TABLEBLOCK_NUM_KEY);

    std::vector<std::pair<std::string, std::string>> db_kv_131;
    auto slash_property_json_parse = json::parse(top::chain_data::stake_property_json);

    auto data = slash_property_json_parse.at(sys_contract_zec_slash_info_addr).at(xstake::XPORPERTY_CONTRACT_UNQUALIFIED_NODE_KEY);
    for (auto _p = data.begin(); _p != data.end(); ++_p)
    {
        db_kv_131.push_back(std::make_pair(base::xstring_utl::base64_decode(_p.key()), base::xstring_utl::base64_decode(_p.value())));
    }

    set_contract_helper(std::make_shared<xcontract_helper>(m_zec_slash_account_ctx_ptr.get(), top::common::xnode_id_t{zec_account.get_account()}, zec_account.get_account()));
    process_reset_data(db_kv_131);

}


TEST_F(test_slash_contract_other, zec_slash_info_summarize) {
    using namespace top::mock;
    xdatamock_unit  zec_account{sys_contract_zec_slash_info_addr};

    m_zec_slash_account_ctx_ptr = make_shared<xaccount_context_t>(zec_account.get_account_state(), m_store.get());
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPORPERTY_CONTRACT_UNQUALIFIED_NODE_KEY);
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPROPERTY_CONTRACT_TABLEBLOCK_NUM_KEY);

    xunqualified_node_info_t  node_info;
    for (auto i = 0; i < 5; ++i) {
        xnode_vote_percent_t node_content;
        node_content.block_count = i + 1;
        node_content.subset_count = i + 1;
        node_info.auditor_info[common::xnode_id_t{"auditor" + std::to_string(i)}] = node_content;
        node_info.validator_info[common::xnode_id_t{"validator" + std::to_string(i)}] = node_content;
    }

    base::xstream_t target_stream(base::xcontext_t::instance());
    node_info.serialize_to(target_stream);
    uint64_t full_tableblock_height = 10;
    target_stream << full_tableblock_height;
    std::string shard_slash_collect = std::string((char*)target_stream.data(), target_stream.size());

    target_stream.reset();
    target_stream << shard_slash_collect;
    auto trx_ptr = summarize_slash_info(std::string((char*) target_stream.data(), target_stream.size()));

    xvm_service vs;
    xtransaction_trace_ptr trace = vs.deal_transaction(trx_ptr, m_zec_slash_account_ctx_ptr.get());
    EXPECT_EQ(enum_xvm_error_code::ok, trace->m_errno);
}

TEST_F(test_slash_contract_other, zec_slash_info_summarize) {
    using namespace top::mock;
    xdatamock_unit  zec_account{sys_contract_zec_slash_info_addr};

    m_zec_slash_account_ctx_ptr = make_shared<xaccount_context_t>(zec_account.get_account_state(), m_store.get());
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPORPERTY_CONTRACT_UNQUALIFIED_NODE_KEY);
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPROPERTY_CONTRACT_TABLEBLOCK_NUM_KEY);

    xunqualified_node_info_t  node_info;
    for (auto i = 0; i < 5; ++i) {
        xnode_vote_percent_t node_content;
        node_content.block_count = i + 1;
        node_content.subset_count = i + 1;
        node_info.auditor_info[common::xnode_id_t{"auditor" + std::to_string(i)}] = node_content;
        node_info.validator_info[common::xnode_id_t{"validator" + std::to_string(i)}] = node_content;
    }

    base::xstream_t target_stream(base::xcontext_t::instance());
    node_info.serialize_to(target_stream);
    uint64_t full_tableblock_height = 10;
    target_stream << full_tableblock_height;
    std::string shard_slash_collect = std::string((char*)target_stream.data(), target_stream.size());

    target_stream.reset();
    target_stream << shard_slash_collect;
    auto trx_ptr = summarize_slash_info(std::string((char*) target_stream.data(), target_stream.size()));

    xvm_service vs;
    xtransaction_trace_ptr trace = vs.deal_transaction(trx_ptr, m_zec_slash_account_ctx_ptr.get());
    EXPECT_EQ(enum_xvm_error_code::ok, trace->m_errno);
}


TEST_F(test_slash_contract_other, zec_slash_do_slash) {
    using namespace top::mock;
    xdatamock_unit  zec_account{sys_contract_zec_slash_info_addr};

    m_zec_slash_account_ctx_ptr = make_shared<xaccount_context_t>(zec_account.get_account_state(), m_store.get());
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPORPERTY_CONTRACT_UNQUALIFIED_NODE_KEY);
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPROPERTY_CONTRACT_TABLEBLOCK_NUM_KEY);
    m_zec_slash_account_ctx_ptr->map_create(xstake::XPROPERTY_CONTRACT_EXTENDED_FUNCTION_KEY);
    m_zec_slash_account_ctx_ptr->map_set(xstake::XPROPERTY_CONTRACT_EXTENDED_FUNCTION_KEY, "SLASH_DELETE_PROPERTY", "false");
    m_zec_slash_account_ctx_ptr->map_set(xstake::XPROPERTY_CONTRACT_EXTENDED_FUNCTION_KEY, "LAST_SLASH_TIME", "0");
    m_zec_slash_account_ctx_ptr->map_set(xstake::XPROPERTY_CONTRACT_EXTENDED_FUNCTION_KEY, "SLASH_TABLE_ROUND", "0");

    auto const time_interval = XGET_ONCHAIN_GOVERNANCE_PARAMETER(punish_interval_time_block);
    auto trx_ptr = do_unqualified_node_slash(time_interval + 1);

    xvm_service vs;
    xtransaction_trace_ptr trace = vs.deal_transaction(trx_ptr, m_zec_slash_account_ctx_ptr.get());
    EXPECT_EQ(enum_xvm_error_code::ok, trace->m_errno);

}
