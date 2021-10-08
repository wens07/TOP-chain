#pragma once

#include "xsystem_contract_runtime/xsystem_contract_manager.h"
#include "xtxpool_service_v2/xtxpool_service_face.h"
#include "xvnetwork/xvnetwork_driver_face.h"
#include "xvnode/xcomponents/xblock_sniffing/xvnode_sniff_config.h"

NS_BEG4(top, vnode, components, sniffing)

enum class enum_vnode_sniff_event_type: uint8_t {
    invalid,
    broadcast,
    timer,
    block,
};
using xvnode_sniff_event_type_t = enum_vnode_sniff_event_type;

enum class enum_vnode_sniff_block_type: uint8_t {
    invalid,
    full_block,
    all
};
using xvnode_sniff_block_type_t = enum_vnode_sniff_block_type;

struct xtop_vnode_sniff_event_config {
    xvnode_sniff_block_type_t type;
    std::function<bool(xobject_ptr_t<base::xvblock_t>)> function;
};
using xvnode_sniff_event_config_t = xtop_vnode_sniff_event_config;
using xvnode_sniff_config_t = std::map<xvnode_sniff_event_type_t, xvnode_sniff_event_config_t>;

NS_END4