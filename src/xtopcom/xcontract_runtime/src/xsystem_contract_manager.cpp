// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_runtime/xsystem_contract_manager.h"

#include "xbasic/xutility.h"
#include "xdata/xblocktool.h"
#include "xdata/xlightunit.h"
#include "xdata/xtransaction.h"
#include "xsystem_contracts/xelection/rec/xrec_standby_pool_contract.h"
#include "xsystem_contracts/xsystem_contract_addresses.h"

using namespace top::base;
using namespace top::common;
using namespace top::data;
using namespace top::system_contracts;

#define REGISTER_ONLY(contract_addr, contract_type)                                                                                                                                \
    {                                                                                                                                                                              \
        xaccount_address_t{contract_addr}, {                                                                                                                                       \
            std::make_shared<contract_type>(), {                                                                                                                                   \
            }                                                                                                                                                                      \
        }                                                                                                                                                                          \
    }

#define REGISTER_DEPLOY(contract_addr, contract_type, sniff_addr, sniff_action, sniff_interval)                                                                                    \
    {                                                                                                                                                                              \
        xaccount_address_t{contract_addr}, {                                                                                                                                       \
            std::make_shared<contract_type>(), {                                                                                                                                   \
                {                                                                                                                                                                  \
                    xaccount_address_t{sniff_addr}, {                                                                                                                              \
                        sniff_action, sniff_interval                                                                                                                               \
                    }                                                                                                                                                              \
                }                                                                                                                                                                  \
            }                                                                                                                                                                      \
        }                                                                                                                                                                          \
    }

NS_BEG2(top, contract_runtime)

void xtop_system_contract_manager::deploy() {
    m_system_contract_deployment_data = {
        REGISTER_ONLY(sys_contract_rec_standby_pool_addr,
                        xrec_standby_pool_contract_new_t),
                        // sys_contract_beacon_timer_addr,
                        // std::bind(&xtop_system_contract_manager::do_on_timer, this, std::placeholders::_1),
                        // 0),
    };
}

void xtop_system_contract_manager::setup(xvblockstore_t * blockstore) {
    for (auto const & contract : m_system_contract_deployment_data) {
        if (is_sys_sharding_contract_address(contract.first)) {
            for (auto i = 0; i < enum_vbucket_has_tables_count; i++) {
                auto addr = make_address_by_prefix_and_subaddr(contract.first.value(), i);
                setup_address(contract.first, addr);
                setup_chain(addr, blockstore);
            }
        } else {
            auto addr = contract.first;
            setup_address(addr, contract.first);
            setup_chain(addr, blockstore);
        }
    }
}

void xtop_system_contract_manager::setup_chain(const xaccount_address_t & cluster_address, xvblockstore_t * blockstore) {
    assert(cluster_address.has_value());

    if (blockstore->exist_genesis_block(cluster_address.value())) {
        xdbg("xtop_contract_manager::setup_chain blockchain account %s genesis block exist", cluster_address.c_str());
        return;
    }
    xdbg("xtop_contract_manager::setup_chain blockchain account %s genesis block not exist", cluster_address.c_str());
#ifdef NEW_VM_SUPPORT
    xtransaction_ptr_t tx = make_object_ptr<xtransaction_t>();
    data::xproperty_asset asset_out{0};
    tx->make_tx_run_contract(asset_out, "setup", "");
    tx->set_same_source_target_address(contract_cluster_address.value());
    tx->set_digest();
    tx->set_len();

    xobject_ptr_t<base::xvbstate_t> bstate = make_object_ptr<base::xvbstate_t>(contract_cluster_address.value(), (uint64_t)0, (uint64_t)0, std::string(), std::string(), (uint64_t)0, (uint32_t)0, (uint16_t)0);
    xaccount_ptr_t unitstate = std::make_shared<xunit_bstate_t>(bstate.get());
    xaccount_context_t ac(unitstate, m_store.get());

    xvm::xvm_service s;
    s.deal_transaction(tx, &ac);

    store::xtransaction_result_t result;
    ac.get_transaction_result(result);

    base::xauto_ptr<base::xvblock_t> block(data::xblocktool_t::create_genesis_lightunit(contract_cluster_address.value(), tx, result));
    xassert(block);

    base::xvaccount_t _vaddr(block->get_account());
    // m_blockstore->delete_block(_vaddr, genesis_block.get());  // delete default genesis block
    auto ret = blockstore->store_block(_vaddr, block.get());
    if (!ret) {
        xerror("xtop_contract_manager::setup_chain %s genesis block fail", contract_cluster_address.c_str());
        return;
    }
    xdbg("[xtop_contract_manager::setup_chain] setup %s, %s", contract_cluster_address.c_str(), ret ? "SUCC" : "FAIL");
#endif
}

void xtop_system_contract_manager::setup_address(const xaccount_address_t & contract_address, const xaccount_address_t & cluster_address) {
    m_rwlock.lock_write();
    auto it = m_contract_inst_map.find(cluster_address);
    if (it == m_contract_inst_map.end()) {
        observer_ptr<xbasic_system_contract_t> pc = system_contract(contract_address);
        if (pc != nullptr) {
            m_contract_inst_map[cluster_address] = pc;
        }
    }
    m_rwlock.release_write();
}

// void xtop_system_contract_manager::setup_sniff() {

// }

bool xtop_system_contract_manager::contains(xaccount_address_t const & address) const noexcept {
    return m_system_contract_deployment_data.find(address) != std::end(m_system_contract_deployment_data);
}

observer_ptr<xbasic_system_contract_t> xtop_system_contract_manager::system_contract(xaccount_address_t const & address) const noexcept {
    auto const it = m_system_contract_deployment_data.find(address);
    if (it != std::end(m_system_contract_deployment_data)) {
        return top::make_observer(top::get<xcontract_deployment_data_t>(*it).m_system_contract.get());
    }

    return nullptr;
}

bool xtop_system_contract_manager::process_sniffed(const xvblock_ptr_t & block) {
    
    // TODO check timer event
    auto const & timer_address = xaccount_address_t{sys_contract_beacon_timer_addr};
    for (auto const & pair : m_system_contract_deployment_data) {
        // TODO check type
        // auto const & action = pair.second.m_sniff_config.get_action(timer_address);
        // action(block);
    }

    return true;
}

// bool xtop_system_contract_manager::do_on_timer(const xaccount_address_t & contract, const xblock_sniff_config_data_t & data, const xvblock_ptr_t & block) {
//     return true;
// }

NS_END2
