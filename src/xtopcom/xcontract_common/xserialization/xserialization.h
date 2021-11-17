// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xutl.h"
#include "xcodec/xmsgpack_codec.hpp"
// #include "xdata/xnative_contract_address.h"
// #include "xvm/xcontract/xcontract_base.h"
// #include "xvm/xerror/xvm_error.h"

#include "xsystem_contracts/xbasic_system_contract.h"

#include <string>

NS_BEG3(top, contract_common, serialization)

extern std::map<common::xaccount_address_t, std::string> const sys_addr_to_metrics_enum_get_property_time;
extern std::map<common::xaccount_address_t, std::string> const sys_addr_to_metrics_enum_set_property_time;
extern std::map<common::xaccount_address_t, std::string> const sys_addr_to_metrics_enum_get_property_size;
extern std::map<common::xaccount_address_t, std::string> const sys_addr_to_metrics_enum_set_property_size;

template <typename T>
struct xtop_msgpack final {
//     static
//     T
//     deserialize_from_string_prop(system_contracts::xbasic_system_contract_t const & contract, std::string const & property_name) {
//         assert(sys_addr_to_metrics_enum_get_property_time.find(contract.address()) != std::end(sys_addr_to_metrics_enum_get_property_time));
//         XMETRICS_TIME_RECORD(xvm::serialization::sys_addr_to_metrics_enum_get_property_time.at(contract.address()));
//         try {
//             //auto string_value = contract.STRING_GET(property_name);
//             std::string string_value;
//             if (string_value.empty()) {
//                 return T{};
//             } else {
//                 XMETRICS_COUNTER_INCREMENT(sys_addr_to_metrics_enum_get_property_size.at(contract.address()), string_value.size());
//                 return codec::msgpack_decode<T>({ std::begin(string_value), std::end(string_value) });
//             }
//         } catch (top::error::xtop_error_t const & eh) {
//             xwarn("[xvm] deserialize %s failed. error category: %s: msg: %s", property_name.c_str(), eh.code().category().name(), eh.what());
//             throw;
//         }
//     }

//     static
//     T
//     deserialize_from_string_prop(system_contracts::xbasic_system_contract_t const & contract,
//                                  std::string const & another_contract_address,
//                                  std::string const & property_name) {
//         assert(sys_addr_to_metrics_enum_get_property_time.find(contract.address()) != std::end(sys_addr_to_metrics_enum_get_property_time));
//         XMETRICS_TIME_RECORD(sys_addr_to_metrics_enum_get_property_time.at(contract.address()));
//         try {
//             // auto string_value = contract.QUERY(xcontract::enum_type_t::string, property_name, "", another_contract_address);
//             std::string string_value;
//             if (string_value.empty()) {
//                 return T{};
//             } else {
//                 XMETRICS_COUNTER_INCREMENT(sys_addr_to_metrics_enum_get_property_size.at(contract.address()), string_value.size());
//                 return codec::msgpack_decode<T>({ std::begin(string_value), std::end(string_value) });
//             }
//         } catch (top::error::xtop_error_t const & eh) {
//             xwarn("[xvm] deserialize %s failed. category: %s; msg: %s", property_name.c_str(), eh.code().category().name(), eh.what());
//             throw;
//         }
//     }

//     static
//     void
//     serialize_to_string_prop(system_contracts::xbasic_system_contract_t & contract, std::string const & property_name, T const & object) {
//         assert(sys_addr_to_metrics_enum_set_property_time.find(contract.address()) != std::end(sys_addr_to_metrics_enum_set_property_time));
//         XMETRICS_TIME_RECORD(sys_addr_to_metrics_enum_set_property_time.at(contract.address()));
//         auto bytes = codec::msgpack_encode(object);
//         std::string obj_str{ std::begin(bytes), std::end(bytes) };
//         XMETRICS_COUNTER_INCREMENT(sys_addr_to_metrics_enum_set_property_size.at(contract.address()), obj_str.size());
//         uint256_t hash = utl::xsha2_256_t::digest((const char*)obj_str.data(), obj_str.size());
//         xinfo("serialize_to_string_prop: %s, %s, %u, %s", typeid(contract).name(), property_name.c_str(), obj_str.size(), data::to_hex_str(hash).c_str());
//         // contract.STRING_SET(property_name, obj_str);
// #if defined DEBUG
//         auto base64str = base::xstring_utl::base64_encode(bytes.data(), static_cast<std::uint32_t>(bytes.size()));
//         xdbg("[serialization] property %s hash %s", property_name.c_str(), base64str.c_str());
// #endif
//     }
    static
    std::string
    serialize_to_string_prop(T const & object) {
        auto bytes = codec::msgpack_encode(object);
        return { std::begin(bytes), std::end(bytes) };
    }

    static
    T
    deserialize_from_string_prop(std::string const & property_value) {
        return codec::msgpack_decode<T>({ std::begin(property_value), std::end(property_value) });
    }
};

template <typename T>
using xmsgpack_t = xtop_msgpack<T>;

NS_END3
