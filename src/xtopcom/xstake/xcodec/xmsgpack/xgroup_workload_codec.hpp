// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xstake/xstake_algorithm.h"

#include <msgpack.hpp>

NS_BEG1(msgpack)
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    NS_BEG1(adaptor)

    XINLINE_CONSTEXPR std::size_t xgroup_workload_field_count{2};
    // XINLINE_CONSTEXPR std::size_t xgroup_workload_group_id_field_index{0};
    XINLINE_CONSTEXPR std::size_t xgroup_workload_group_total_workload_field_index{0};
    XINLINE_CONSTEXPR std::size_t xgroup_workload_leader_count_field_index{1};

    template <>
    struct convert<top::xstake::xgroup_workload_t> final {
        msgpack::object const & operator()(msgpack::object const & o, top::xstake::xgroup_workload_t & v) const {
            if (o.type != msgpack::type::ARRAY) {
                throw msgpack::type_error{};
            }

            if (o.via.array.size == 0) {
                return o;
            }

            switch (o.via.array.size - 1) {
            default:
                XATTRIBUTE_FALLTHROUGH;

            case xgroup_workload_leader_count_field_index:
                v.m_leader_count = o.via.array.ptr[xgroup_workload_leader_count_field_index].as<std::map<std::string, uint32_t>>();
                XATTRIBUTE_FALLTHROUGH;

            case xgroup_workload_group_total_workload_field_index:
                v.cluster_total_workload = o.via.array.ptr[xgroup_workload_group_total_workload_field_index].as<uint32_t>();
                XATTRIBUTE_FALLTHROUGH;

            //case xgroup_workload_group_id_field_index:
            //    v.cluster_id = o.via.array.ptr[xgroup_workload_group_id_field_index].as<std::string>();
            //    XATTRIBUTE_FALLTHROUGH;
            }

            return o;
        }
    };

    template <>
    struct pack<top::xstake::xgroup_workload_t> {
        template <typename Stream>
        msgpack::packer<Stream> & operator()(msgpack::packer<Stream> & o, top::xstake::xgroup_workload_t const & message) const {
            o.pack_array(xgroup_workload_field_count);
            // o.pack(message.cluster_id);
            o.pack(message.cluster_total_workload);
            o.pack(message.m_leader_count);

            return o;
        }
    };

    //template <>
    //struct object_with_zone<top::xstake::xgroup_workload_t> {
    //    void operator()(msgpack::object::with_zone & o, top::xstake::xgroup_workload_t const & message) const {
    //        o.type = type::ARRAY;
    //        o.via.array.size = xgroup_workload_field_count;
    //        o.via.array.ptr = static_cast<msgpack::object *>(o.zone.allocate_align(sizeof(msgpack::object) * o.via.array.size));

    //        o.via.array.ptr[xgroup_workload_group_id_field_index] = msgpack::object{message.xip(), o.zone};
    //    }
    //};

    NS_END1
}
NS_END1
