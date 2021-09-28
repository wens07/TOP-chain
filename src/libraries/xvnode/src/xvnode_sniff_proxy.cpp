// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xvnode/xvnode_sniff_proxy.h"

#include "xmbus/xevent_store.h"
#include "xmbus/xevent_timer.h"

NS_BEG2(top, vnode)

xtop_vnode_sniff_proxy::xtop_vnode_sniff_proxy(observer_ptr<mbus::xmessage_bus_face_t> const & bus) : mbus(bus) {
}

void xtop_vnode_sniff_proxy::start() {
    m_store_event_id = mbus->add_listener(mbus::xevent_major_type_store, std::bind(&xtop_vnode_sniff_proxy::sniff, this, std::placeholders::_1));
    m_timer_event_id = mbus->add_listener(mbus::xevent_major_type_chain_timer, std::bind(&xtop_vnode_sniff_proxy::sniff, this, std::placeholders::_1));
}

void xtop_vnode_sniff_proxy::stop() {
    assert(m_store_event_id != INVALID_EVENT_ID);
    assert(m_timer_event_id != INVALID_EVENT_ID);
    mbus->remove_listener(mbus::xevent_major_type_store, m_store_event_id);
    mbus->remove_listener(mbus::xevent_major_type_chain_timer, m_timer_event_id);
    m_store_event_id = INVALID_EVENT_ID;
    m_timer_event_id = INVALID_EVENT_ID;
    m_sniff_config.clear();
}

void xtop_vnode_sniff_proxy::reg(common::xnode_address_t const & address, xvnode_sniff_config_t const & config) {
    m_sniff_config.insert(std::make_pair(address, config));
}

void xtop_vnode_sniff_proxy::unreg(common::xnode_address_t const & address) {
    m_sniff_config.erase(address);
}

void xtop_vnode_sniff_proxy::sniff(mbus::xevent_ptr_t const & e) {
    xdbg("xtop_contract_manager::process_event %d", e->major_type);
    if (e->major_type == mbus::xevent_major_type_chain_timer) {
        auto const & event = dynamic_xobject_ptr_cast<mbus::xevent_chain_timer_t>(e);
        auto const & vblock = make_object_ptr<base::xvblock_t>(*(event->time_block));
        assert(vblock != nullptr);

        for (auto const & config : m_sniff_config) {
            auto it = config.second.find(xvnode_sniff_event_type_t::timer);
            if (it != config.second.end()) {
                it->second.function(vblock);
            }
        }
    } else if (e->major_type == mbus::xevent_major_type_store) {
        auto const & event = dynamic_xobject_ptr_cast<mbus::xevent_store_block_committed_t>(e);
        if (event->minor_type != mbus::xevent_store_t::type_block_committed) {
            return;
        }
        if (event->blk_level != base::enum_xvblock_level_table) {
            return;
        }
        if (event->blk_class != base::enum_xvblock_class_full) {
            return;
        }
        auto const & vblock = mbus::extract_block_from(event, true, metrics::blockstore_access_from_mbus_contract_db_on_block);  // load mini-block firstly
        assert(vblock != nullptr);

        xdbg("[xtop_vnode_sniff_proxy::sniff] committed table block to db, block=%s", vblock->dump().c_str());

        for (auto const & config : m_sniff_config) {
            auto it = config.second.find(xvnode_sniff_event_type_t::block);
            if (it != config.second.end()) {
                it->second.function(vblock);
            }
        }
    }
}

NS_END2