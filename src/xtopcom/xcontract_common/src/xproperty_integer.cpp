// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_common/xproperties/xproperty_integer.h"

NS_BEG3(top, contract_common, properties)

#define DEFINE_INT_PROPERTY(INT_TYPE)   \
    xtop_##INT_TYPE##_property::xtop_##INT_TYPE##_property(std::string const & name, xcontract_face_t * contract)                                                               \
        : xbasic_property_t {name, state_accessor::properties::xproperty_type_t::INT_TYPE, make_observer(contract)} {                                                           \
    }                                                                                                                                                                           \
                                                                                                                                                                                \
    void xtop_##INT_TYPE##_property::set(INT_TYPE##_t const value) {                                                                                                            \
        m_associated_contract->contract_state()->set_property<state_accessor::properties::xproperty_type_t::INT_TYPE>(                                                          \
        static_cast<state_accessor::properties::xtypeless_property_identifier_t>(m_id), value);                                                                                 \
    }                                                                                                                                                                           \
                                                                                                                                                                                \
    void xtop_##INT_TYPE##_property::clear() {                                                                                                                                  \
        m_associated_contract->contract_state()->clear_property(m_id);                                                                                                          \
    }                                                                                                                                                                           \
                                                                                                                                                                                \
    INT_TYPE##_t xtop_##INT_TYPE##_property::value() const {                                                                                                                    \
        return m_associated_contract->contract_state()->get_property<state_accessor::properties::xproperty_type_t::INT_TYPE>(                                                   \
        static_cast<state_accessor::properties::xtypeless_property_identifier_t>(m_id));                                                                                        \
    }

DEFINE_INT_PROPERTY(uint64)
DEFINE_INT_PROPERTY(int64)

#undef DEFINE_INT_PROPERTY

NS_END3