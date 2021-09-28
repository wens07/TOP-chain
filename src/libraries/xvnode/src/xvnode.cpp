// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xvnode/xvnode.h"

#include "xdata/xfull_tableblock.h"
#include "xmbus/xevent_role.h"
#include "xvm/manager/xcontract_address_map.h"
#include "xvm/manager/xcontract_manager.h"
#include "xvnetwork/xvnetwork_driver.h"
#include "xvnode/xerror/xerror.h"

NS_BEG2(top, vnode)

xtop_vnode::xtop_vnode(observer_ptr<elect::ElectMain> const & elect_main,
                       common::xsharding_address_t const & sharding_address,
                       common::xslot_id_t const & slot_id,
                       common::xelection_round_t joined_election_round,
                       common::xelection_round_t election_round,
                       std::uint16_t const group_size,
                       std::uint64_t const associated_blk_height,
                       observer_ptr<vnetwork::xvhost_face_t> const & vhost,
                       observer_ptr<router::xrouter_face_t> const & router,
                       observer_ptr<store::xstore_face_t> const & store,
                       observer_ptr<base::xvblockstore_t> const & block_store,
                       observer_ptr<mbus::xmessage_bus_face_t> const & bus,
                       observer_ptr<time::xchain_time_face_t> const & logic_timer,
                       observer_ptr<sync::xsync_object_t> const & sync_obj,
                       observer_ptr<grpcmgr::xgrpc_mgr_t> const & grpc_mgr,
                    //    observer_ptr<xunit_service::xcons_service_mgr_face> const & cons_mgr,
                       observer_ptr<xtxpool_service_v2::xtxpool_service_mgr_face> const & txpool_service_mgr,
                       observer_ptr<xtxpool_v2::xtxpool_face_t> const & txpool,
                       observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor,
                       observer_ptr<xbase_timer_driver_t> const & timer_driver,
                       xobject_ptr_t<base::xvnodesrv_t> const & nodesvr)
  : xbasic_vnode_t{common::xnode_address_t{sharding_address,
                                           common::xaccount_election_address_t{vhost->host_node_id(), slot_id},
                                           election_round,
                                           group_size,
                                           associated_blk_height},
                   joined_election_round,
                   vhost,
                   election_cache_data_accessor}
  , m_elect_main{elect_main}
  , m_router{router}
  , m_store{store}
  , m_block_store{block_store}
  , m_bus{bus}
  , m_logic_timer{logic_timer}
  , m_sync_obj{sync_obj}
  , m_grpc_mgr{grpc_mgr}
  , m_txpool{txpool}
  , m_dev_params{make_observer(std::addressof(data::xdev_params::get_instance()))}
  , m_user_params{make_observer(std::addressof(data::xuser_params::get_instance()))}
  , m_the_binding_driver{std::make_shared<vnetwork::xvnetwork_driver_t>(
        m_vhost, m_election_cache_data_accessor,
        common::xnode_address_t{sharding_address, common::xaccount_election_address_t{m_vhost->host_node_id(), slot_id}, election_round, group_size, associated_blk_height},
        joined_election_round)}
  , m_timer_driver{timer_driver}
  , m_tx_prepare_mgr{nullptr}
  , m_nodesvr{nodesvr}
  , m_system_contract_manager{make_observer(contract_runtime::system::xsystem_contract_manager_t::instance())} {
    bool is_edge_archive = common::has<common::xnode_type_t::storage>(m_the_binding_driver->type()) || common::has<common::xnode_type_t::edge>(m_the_binding_driver->type());
    bool is_frozen = common::has<common::xnode_type_t::frozen>(m_the_binding_driver->type());
    if (!is_edge_archive && !is_frozen) {
        // m_cons_face = cons_mgr->create(m_the_binding_driver);
        m_txpool_face = txpool_service_mgr->create(m_the_binding_driver, m_router);

        // xwarn("[virtual node] vnode %p create at address %s cons_proxy:%p txproxy:%p",
        //       this,
        //       m_the_binding_driver->address().to_string().c_str(),
        //       m_cons_face.get(),
        //       m_txpool_face.get());
    } else {
        xwarn("[virtual node] vnode %p create at address %s", this, m_the_binding_driver->address().to_string().c_str());
    }

    set_role_data();
}

xtop_vnode::xtop_vnode(observer_ptr<elect::ElectMain> const & elect_main,
                       observer_ptr<vnetwork::xvhost_face_t> const & vhost,
                       std::shared_ptr<election::cache::xgroup_element_t> group_info,
                       observer_ptr<router::xrouter_face_t> const & router,
                       observer_ptr<store::xstore_face_t> const & store,
                       observer_ptr<base::xvblockstore_t> const & block_store,
                       observer_ptr<mbus::xmessage_bus_face_t> const & bus,
                       observer_ptr<time::xchain_time_face_t> const & logic_timer,
                       observer_ptr<sync::xsync_object_t> const & sync_obj,
                       observer_ptr<grpcmgr::xgrpc_mgr_t> const & grpc_mgr,
                    //    observer_ptr<xunit_service::xcons_service_mgr_face> const & cons_mgr,
                       observer_ptr<xtxpool_service_v2::xtxpool_service_mgr_face> const & txpool_service_mgr,
                       observer_ptr<xtxpool_v2::xtxpool_face_t> const & txpool,
                       observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor,
                       observer_ptr<xbase_timer_driver_t> const & timer_driver,
                       xobject_ptr_t<base::xvnodesrv_t> const & nodesvr)
  : xtop_vnode{elect_main,
               group_info->node_element(vhost->host_node_id())->address().sharding_address(),
               group_info->node_element(vhost->host_node_id())->slot_id(),
               group_info->node_element(vhost->host_node_id())->joined_election_round(),
               group_info->election_round(),
               group_info->group_size(),
               group_info->associated_blk_height(),
               vhost,
               router,
               store,
               block_store,
               bus,
               logic_timer,
               sync_obj,
               grpc_mgr,
            //    cons_mgr,
               txpool_service_mgr,
               txpool,
               election_cache_data_accessor,
               timer_driver,
               nodesvr} {}

std::shared_ptr<vnetwork::xvnetwork_driver_face_t> const & xtop_vnode::vnetwork_driver() const noexcept {
    return m_the_binding_driver;
}

void xtop_vnode::synchronize() {
    if (m_sync_started)
        return;

    sync_add_vnet();

    m_sync_started = true;

    m_the_binding_driver->start();

    xinfo("[virtual node] vnode (%p) start synchronizing at address %s", this, m_the_binding_driver->address().to_string().c_str());
}

void xtop_vnode::start() {
    assert(!running());

    assert(m_the_binding_driver != nullptr);
    assert(m_router != nullptr);
    assert(m_logic_timer != nullptr);
    assert(m_vhost != nullptr);

    new_driver_added();
    m_grpc_mgr->try_add_listener(common::has<common::xnode_type_t::storage_archive>(vnetwork_driver()->type()) ||
        common::has<common::xnode_type_t::storage_full_node>(vnetwork_driver()->type()));
    // if (m_cons_face != nullptr) {
    //     m_cons_face->start(this->start_time());
    // }
    if (m_txpool_face != nullptr) {
        m_txpool_face->start();
    }

    running(true);
    xkinfo("[virtual node] vnode (%p) start running at address %s", this, m_the_binding_driver->address().to_string().c_str());
}

void xtop_vnode::fade() {
    assert(running());
    assert(!faded());
    assert(m_the_binding_driver != nullptr);
    assert(m_the_binding_driver->running());

    update_contract_manager(true);

    sync_remove_vnet();
    // if (m_cons_face != nullptr) {
    //     m_cons_face->fade();
    // }
    m_faded.store(true, std::memory_order_release);
}

void xtop_vnode::stop() {
    assert(running());
    assert(faded());
    // any component can stop should
    // control multi-times stop
    if (m_txpool_face != nullptr) {
        m_txpool_face->unreg();
    }
    m_grpc_mgr->try_remove_listener(common::has<common::xnode_type_t::storage_archive>(vnetwork_driver()->type()));
    running(false);
    m_the_binding_driver->stop();
    driver_removed();
    update_contract_manager(true);

    xkinfo("[virtual node] vnode (%p) stop running at address %s", this, m_the_binding_driver->address().to_string().c_str());
}

void xtop_vnode::new_driver_added() {
    // need call by order, if depends on other components
    // for example : store depends on message bus, then
    // update_message_bus should be called before update_store
    // update_consensus_instance();
    // update_unit_service();
    update_tx_cache_service();
    update_rpc_service();
    update_contract_manager(false);
    update_block_prune();
}

void xtop_vnode::driver_removed() {
    if (m_rpc_services != nullptr) {
        m_rpc_services->stop();
    }
    if (common::has<common::xnode_type_t::storage_full_node>(m_the_binding_driver->type()) && m_tx_prepare_mgr != nullptr) {
        m_tx_prepare_mgr->stop();
    }
    sync_remove_vnet();
}
void xtop_vnode::update_block_prune() {
    xdbg("try update block prune. node type %s, %p", common::to_string(m_the_binding_driver->type()).c_str(), m_timer_driver.get());
    if (XGET_CONFIG(auto_prune_data) == "off")
        return;
    if (common::has<common::xnode_type_t::frozen>(m_the_binding_driver->type())) {
        return;
    }
    if (!common::has<common::xnode_type_t::storage>(m_the_binding_driver->type())) {
        if (top::store::install_block_recycler(m_store.get()))
            xdbg("install_block_recycler ok.");
        else
            xdbg("install_block_recycler fail.");
        if (top::store::enable_block_recycler())
            xdbg("enable_block_recycler ok.");
        else
            xdbg("enable_block_recycler fail.");
    }
}
void xtop_vnode::update_rpc_service() {
    xdbg("try update rpc service. node type %s", common::to_string(m_the_binding_driver->type()).c_str());
    if (!common::has<common::xnode_type_t::storage_archive>(m_the_binding_driver->type()) &&
        !common::has<common::xnode_type_t::frozen>(m_the_binding_driver->type())) {
        auto const http_port = XGET_CONFIG(http_port);
        auto const ws_port = XGET_CONFIG(ws_port);
        // TODO(justin): remove unit_services temp
        xdbg("[virtual node] update rpc service with node type %s address %s",
             common::to_string(m_the_binding_driver->type()).c_str(),
             m_the_binding_driver->address().to_string().c_str());
        m_rpc_services = std::make_shared<xrpc::xrpc_init>(
            m_the_binding_driver, m_the_binding_driver->type(), m_router, http_port, ws_port,
            m_txpool_face, m_store, m_block_store, m_elect_main, m_election_cache_data_accessor, make_observer(m_transaction_cache));
    }
}
void xtop_vnode::update_tx_cache_service() {
    xdbg("try update tx cache service. node type %s, %p", common::to_string(m_the_binding_driver->type()).c_str(), m_timer_driver.get());
    if (common::has<common::xnode_type_t::storage_full_node>(m_the_binding_driver->type())) {
        xdbg("[virtual node] update tx cache service with node type %s address %s",
             common::to_string(m_the_binding_driver->type()).c_str(),
             m_the_binding_driver->address().to_string().c_str());
        m_transaction_cache = std::make_shared<data::xtransaction_cache_t>();
        m_tx_prepare_mgr = std::make_shared<txexecutor::xtransaction_prepare_mgr>(m_bus, m_timer_driver, make_observer(m_transaction_cache));
        m_tx_prepare_mgr->start();
    }
}

void xtop_vnode::update_contract_manager(bool destory) {
    // TODO(justin): remove unit_services temp
    contract::xcontract_manager_t::instance().push_event(make_object_ptr<mbus::xevent_vnode_t>(destory, m_txpool_face, m_the_binding_driver));
}

void xtop_vnode::sync_add_vnet() {
    assert(!m_sync_started);

    m_sync_obj->add_vnet(vnetwork_driver());

    xinfo("vnode (%p) at address %s starts synchronizing", this, address().to_string().c_str());
}

void xtop_vnode::sync_remove_vnet() {
    // if ((type() & common::xnode_type_t::edge) == common::xnode_type_t::invalid) {

    m_sync_obj->remove_vnet(vnetwork_driver());

    //}
}

//std::vector<common::xip2_t> get_group_nodes_xip2_from(std::shared_ptr<xvnode_face_t> const & vnode, common::xip_t const & group_xip, std::error_code & ec) const {
//    assert(!ec);
//
//    if (address().xip2().xip().group_xip() == group_xip) {
//        return neighbors_xip2(ec);
//    }
//}

void xtop_vnode::set_role_data() {
    // common::xnode_type_t type = e->driver->type();
    // common::xnode_type_t type = m_the_binding_driver->type();
    common::xnode_type_t type{m_the_binding_driver->type()};
    bool disable_broadcasts{false};
    xdbg("[xtop_vnode::get_roles_data] node type : %s", common::to_string(type).c_str());

    if (common::has<common::xnode_type_t::consensus_auditor>(type)) {
        xdbg("[xtop_vnode::get_roles_data] add all sharding contracts' rcs");
        type = common::xnode_type_t::consensus_validator;
        disable_broadcasts = true;
    }

    auto const & system_contract_deployment_data = m_system_contract_manager->deployment_data();
    for (auto const & contract_data_pair : system_contract_deployment_data) {
        auto const & contract_address = contract_data_pair.first;
        auto const & contract_data = contract_data_pair.second;
        
        if (!common::has(type, contract_data.m_node_type)) {
            continue;
        }
        if (!m_role_data.count(contract_address)) {
            m_role_data.insert(std::make_pair(contract_address, xvnode_role_config_t{contract_data, std::map<common::xaccount_address_t, uint64_t>()}));
        }
        if (disable_broadcasts) {
            m_role_data[contract_address].m_role_config.m_broadcast_config.m_type = contract_runtime::xsniff_broadcast_type_t::invalid;
        }
    }
#if defined(DEBUG)
    for (auto const & data_pair : m_role_data) {
        xdbg("address: %s, driver type: %d", data_pair.first.c_str(), m_the_binding_driver->type());
        auto const & data = data_pair.second;
        xdbg("contract: %p, node type: %d, sniff type: %d, broadcast: %d, %d, timer: %d, action: %s",
             &data.m_role_config.m_system_contract,
             data.m_role_config.m_node_type,
             data.m_role_config.m_sniff_type,
             data.m_role_config.m_broadcast_config.m_type,
             data.m_role_config.m_broadcast_config.m_policy,
             data.m_role_config.m_timer_config.m_interval,
             data.m_role_config.m_timer_config.m_action.c_str());
    }
#endif
}

xvnode_sniff_config_t xtop_vnode::sniff_config() {
    xvnode_sniff_config_t config;
    for (auto const & data_pair : m_role_data) {
        auto const & data = data_pair.second;
        auto const & sniff_type = data.m_role_config.m_sniff_type;
        if (static_cast<uint32_t>(contract_runtime::xsniff_type_t::broadcast) & static_cast<uint32_t>(sniff_type)) {
            config.insert(std::make_pair(xvnode_sniff_event_type_t::broadcast,
                                         xvnode_sniff_event_config_t{xvnode_sniff_block_type_t::full_block, std::bind(&xtop_vnode::sniff_broadcast, this, std::placeholders::_1)}));
        } else if (static_cast<uint32_t>(contract_runtime::xsniff_type_t::timer) & static_cast<uint32_t>(sniff_type)) {
            config.insert(std::make_pair(xvnode_sniff_event_type_t::timer,
                                         xvnode_sniff_event_config_t{xvnode_sniff_block_type_t::all, std::bind(&xtop_vnode::sniff_timer, this, std::placeholders::_1)}));
        } else if (static_cast<uint32_t>(contract_runtime::xsniff_type_t::block) & static_cast<uint32_t>(sniff_type)) {
            config.insert(std::make_pair(xvnode_sniff_event_type_t::block,
                                         xvnode_sniff_event_config_t{xvnode_sniff_block_type_t::full_block, std::bind(&xtop_vnode::sniff_block, this, std::placeholders::_1)}));
        }
    }

    return config;
}

bool xtop_vnode::sniff_broadcast(xobject_ptr_t<base::xvblock_t> const & vblock) {
    return true;
}

bool xtop_vnode::sniff_timer(xobject_ptr_t<base::xvblock_t> const & vblock) {
    auto const height = vblock->get_height();
    for (auto & role_data_pair : m_role_data) {
        auto const & contract_address = role_data_pair.first;
        auto const & config = role_data_pair.second.m_role_config;
        if ((static_cast<uint32_t>(contract_runtime::xsniff_type_t::timer) & static_cast<uint32_t>(config.m_sniff_type)) == 0) {
            continue;
        }
        xdbg("[xtop_vnode::sniff_timer] block address: %s, height: %" PRIu64, vblock->get_account().c_str(), vblock->get_height());
        assert(common::xaccount_address_t{vblock->get_account()} == common::xaccount_address_t{sys_contract_beacon_timer_addr});
        auto valid = is_valid_timer_call(contract_address, role_data_pair.second, height);
        xdbg("[xtop_vnode::sniff_timer] contract address %s, interval: %u, valid: %d", contract_address.c_str(), config.m_timer_config.m_interval, valid);
        if (!valid) {
            return false;
        }
        base::xstream_t stream(base::xcontext_t::instance());
        stream << height;
        std::string action_params = std::string((char *)stream.data(), stream.size());
        xdbg("[xtop_vnode::sniff_timer] make tx, action: %s, params: %s", config.m_timer_config.m_action.c_str(), action_params.c_str());
        call(contract_address, config.m_timer_config.m_action, action_params, vblock->get_cert()->get_gmtime());
    }
    return true;
}

bool xtop_vnode::sniff_block(xobject_ptr_t<base::xvblock_t> const & vblock) {
    auto const & block_address = vblock->get_account();
    auto const height = vblock->get_height();
    for (auto & role_data_pair : m_role_data) {
        auto const & contract_address = role_data_pair.first;
        auto const & config = role_data_pair.second.m_role_config;
        if ((static_cast<uint32_t>(contract_runtime::xsniff_type_t::block) & static_cast<uint32_t>(config.m_sniff_type)) == 0) {
            continue;
        }

        // table upload contract sniff sharding table addr
        if ((block_address.find(sys_contract_sharding_table_block_addr) != std::string::npos) && (contract_address.value() == sys_contract_sharding_statistic_info_addr)) {
            xdbg("[xtop_vnode::sniff_block] sniff block match, contract: %s, block: %s, height: %" PRIu64, contract_address.c_str(), block_address.c_str(), height);
            return true;
            auto const full_tableblock = (dynamic_cast<xfull_tableblock_t *>(vblock.get()));
            auto const fulltable_statisitc_data_str = full_tableblock->get_table_statistics_string();

            base::xstream_t stream(base::xcontext_t::instance());
            stream << fulltable_statisitc_data_str;
            stream << height;
            stream << full_tableblock->get_pledge_balance_change_tgas();
            std::string action_params = std::string((char *)stream.data(), stream.size());
            uint32_t table_id = 0;
            auto result = xdatautil::extract_table_id_from_address(block_address, table_id);
            assert(result);
            {
                // table id check
                auto const & driver_ids = m_the_binding_driver->table_ids();
                auto result = find(driver_ids.begin(), driver_ids.end(), table_id);
                if (result == driver_ids.end()) {
                    return false;
                }
            }
            auto const & table_address = contract::xcontract_address_map_t::calc_cluster_address(contract_address, table_id);

            XMETRICS_GAUGE(metrics::xmetircs_tag_t::contract_table_fullblock_event, 1);
            call(table_address, config.m_block_config.m_action, action_params, vblock->get_cert()->get_gmtime());
        }
    }
    return true;
}

bool xtop_vnode::is_valid_timer_call(common::xaccount_address_t const & address, xvnode_role_config_t & data, const uint64_t height) const {
    static std::vector<common::xaccount_address_t> sys_addr_list{common::xaccount_address_t{sys_contract_rec_elect_edge_addr},
                                                                 common::xaccount_address_t{sys_contract_rec_elect_archive_addr},
                                                                 // common::xaccount_address_t{ sys_contract_zec_elect_edge_addr },
                                                                 // common::xaccount_address_t{ sys_contract_zec_elect_archive_addr },
                                                                 common::xaccount_address_t{sys_contract_rec_elect_zec_addr},
                                                                 common::xaccount_address_t{sys_contract_zec_elect_consensus_addr}};
    bool is_first_block{false};
    if (std::find(std::begin(sys_addr_list), std::end(sys_addr_list), address) != std::end(sys_addr_list)) {
        if (m_store->query_account(address.value())->get_chain_height() == 0) {
            is_first_block = true;
        }
    }

    auto const interval = data.m_role_config.m_timer_config.m_interval;
    assert(interval > 0);
    if (interval != 0 && height != 0 && ((is_first_block && (height % 3) == 0) || (!is_first_block && (height % interval) == 0))) {
        xdbg("[xtop_vnode::is_valid_timer_call] param check pass, interval: %u, height: %llu, first_block: %d", interval, height, is_first_block);
    } else {
        xdbg("[xtop_vnode::is_valid_timer_call] param check not pass, interval: %u, height: %llu, first_block: %d", interval, height, is_first_block);
        return false;
    }

    auto & round = data.m_address_round;
    auto iter = round.find(address);
    if (iter == round.end() || (iter != round.end() && iter->second < height)) {
        round[address] = height;
        return true;
    } else {
        xwarn("[xtop_vnode::is_valid_timer_call] address %s height check error, last height: %llu, this height : %llu", address.c_str(), round[address], height);
        return false;
    }
}

void xtop_vnode::call(common::xaccount_address_t const & address, std::string const & action_name, std::string const & action_params, const uint64_t timestamp) {
    xproperty_asset asset_out{0};
    auto tx = make_object_ptr<xtransaction_v2_t>();

    tx->make_tx_run_contract(asset_out, action_name, action_params);
    tx->set_same_source_target_address(address.value());
    xaccount_ptr_t account = m_store->query_account(address.value());
    assert(account != nullptr);
    tx->set_last_trans_hash_and_nonce(account->account_send_trans_hash(), account->account_send_trans_number());
    tx->set_fire_timestamp(timestamp);
    tx->set_expire_duration(300);
    tx->set_digest();
    tx->set_len();

    int32_t r = m_txpool_face->request_transaction_consensus(tx, true);
    xinfo("[xrole_context_t] call_contract in consensus mode with return code : %d, %s, %s %s %ld, %lld",
          r,
          tx->get_digest_hex_str().c_str(),
          address.value().c_str(),
          data::to_hex_str(account->account_send_trans_hash()).c_str(),
          account->account_send_trans_number(),
          timestamp);
}

void xtop_vnode::call(common::xaccount_address_t const & source_address,
                      common::xaccount_address_t const & target_address,
                      std::string const & action_name,
                      std::string const & action_params,
                      uint64_t timestamp) {
    auto tx = make_object_ptr<xtransaction_v2_t>();
    tx->make_tx_run_contract(action_name, action_params);
    tx->set_different_source_target_address(source_address.value(), target_address.value());
    xaccount_ptr_t account = m_store->query_account(source_address.value());
    assert(account != nullptr);
    tx->set_last_trans_hash_and_nonce(account->account_send_trans_hash(), account->account_send_trans_number());
    tx->set_fire_timestamp(timestamp);
    tx->set_expire_duration(300);
    tx->set_digest();
    tx->set_len();

    int32_t r = m_txpool_face->request_transaction_consensus(tx, true);
    xinfo("[xrole_context_t::fulltableblock_event] call_contract in consensus mode with return code : %d, %s, %s %s %ld, %lld",
            r,
            tx->get_digest_hex_str().c_str(),
            source_address.c_str(),
            data::to_hex_str(account->account_send_trans_hash()).c_str(),
            account->account_send_trans_number(),
            timestamp);
}

NS_END2
