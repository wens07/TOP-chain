// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbasic/xmemory.hpp"
#include "xbasic/xtimer_driver_fwd.h"
#include "xelect_net/include/elect_main.h"
#include "xelection/xcache/xdata_accessor_face.h"
#include "xelection/xcache/xgroup_element.h"
#include "xgrpc_mgr/xgrpc_mgr.h"
#include "xmbus/xmessage_bus.h"
#include "xrouter/xrouter_face.h"
#include "xsync/xsync_object.h"
#include "xtxpool_service_v2/xtxpool_service_face.h"
#include "xtxpool_v2/xtxpool_face.h"
#include "xunit_service/xcons_face.h"
#include "xvnetwork/xmessage_callback_hub.h"
#include "xvnode/xvnode_face.h"
#include "xvnode/xvnode_factory_face.h"
#include "xvnode/xvnode_manager_face.h"
#include "xvnode/xvnode_role_proxy_face.h"
#include "xvnode/xvnode_sniff_proxy_face.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

NS_BEG2(top, vnode)

class xtop_vnode_manager : public xvnode_manager_face_t
                         , public std::enable_shared_from_this<xtop_vnode_manager> {
private:
    using xbase_t = xvnode_manager_face_t;

    observer_ptr<time::xchain_time_face_t> m_logic_timer;
    observer_ptr<vnetwork::xvhost_face_t> m_vhost;
    std::unique_ptr<xvnode_factory_face_t> m_vnode_factory;
    std::unique_ptr<xvnode_role_proxy_face_t> m_vnode_proxy;
    std::unique_ptr<xvnode_sniff_proxy_face_t> m_sniff_proxy;

    std::shared_ptr<vnetwork::xmessage_callback_hub_t> m_message_callback_hub{};
    std::mutex m_nodes_mutex{};
    std::unordered_map<common::xnode_address_t, std::shared_ptr<xvnode_face_t>> m_all_nodes{};

#if defined(DEBUG)
    std::thread::id m_election_notify_thread_id{};
    std::mutex m_temp_mutex;
#endif

    static constexpr char const * chain_timer_watch_name = "virtual_node_mgr_on_timer";

public:
    xtop_vnode_manager(xtop_vnode_manager const &) = delete;
    xtop_vnode_manager & operator=(xtop_vnode_manager const &) = delete;
    xtop_vnode_manager(xtop_vnode_manager &&) = default;
    xtop_vnode_manager & operator=(xtop_vnode_manager &&) = default;
    ~xtop_vnode_manager() = default;

    xtop_vnode_manager(observer_ptr<elect::ElectMain> const & elect_main,
                       observer_ptr<mbus::xmessage_bus_face_t> const & mbus,
                       observer_ptr<store::xstore_face_t> const & store,
                       observer_ptr<base::xvblockstore_t> const & block_store,
                       observer_ptr<time::xchain_time_face_t> const & logic_timer,
                       observer_ptr<router::xrouter_face_t> const & router,
                       xobject_ptr_t<base::xvcertauth_t> const & certauth,
                       observer_ptr<vnetwork::xvhost_face_t> const & vhost,
                       observer_ptr<sync::xsync_object_t> const & sync_object,
                       observer_ptr<grpcmgr::xgrpc_mgr_t> const & grpc_mgr,
                    //    observer_ptr<xunit_service::xcons_service_mgr_face> const & cons_mgr,
                       observer_ptr<xtxpool_service_v2::xtxpool_service_mgr_face> const & txpool_service_mgr,
                       observer_ptr<xtxpool_v2::xtxpool_face_t> const & txpool,
                       observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor,
                       observer_ptr<xbase_timer_driver_t> const & timer_driver);

    xtop_vnode_manager(observer_ptr<time::xchain_time_face_t> const & logic_timer,
                       observer_ptr<vnetwork::xvhost_face_t> const & vhost,
                       std::unique_ptr<xvnode_factory_face_t> vnode_factory,
                       std::unique_ptr<xvnode_role_proxy_face_t> vnode_proxy,
                       std::unique_ptr<xvnode_sniff_proxy_face_t> sniff_proxy);

    void start() override;

    void stop() override;

    /**
     * @return std::pair<vector<common::xip2_t>, vector<common::xip2_t>> 
     * 
     * Pair.first is purely outdated xip2, which contains nodes which should be outdated at this exact version.
     * Pair.second is logical outdated xip2,which containes nodes which were not in group for two continuous versions.
     * 
     * Certainly pair.second is the subset of pair.first 
     */
    std::pair<std::vector<common::xip2_t>, std::vector<common::xip2_t>> handle_election_data(
        std::unordered_map<common::xsharding_address_t, election::cache::xgroup_update_result_t> const & election_data) override;

private:
    void on_timer(common::xlogic_time_t time);
};
using xvnode_manager_t = xtop_vnode_manager;

NS_END2
