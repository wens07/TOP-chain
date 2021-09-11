// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_common/xproperties/xproperty_string.h"

NS_BEG3(top, contract_common, properties)

xtop_string_property::xtop_string_property(std::string const & prop_name, contract_common::xbasic_contract_t * contract)
  : xbasic_property_t{prop_name, state_accessor::properties::xproperty_type_t::string, make_observer(contract)} {
}

void xtop_string_property::create() {
    m_associated_contract->state()->access_control()->string_prop_create(accessor(), m_id);
}

void xtop_string_property::update(std::string const & prop_value) {
    m_associated_contract->state()->access_control()->string_prop_update(accessor(), m_id, prop_value);
}

void xtop_string_property::clear() {
    m_associated_contract->state()->access_control()->string_prop_clear(accessor(), m_id);
}

std::string xtop_string_property::query() const {
    return m_associated_contract->state()->access_control()->string_prop_query(accessor(), m_id);
}

std::string xtop_string_property::query(common::xaccount_address_t const & contract) const {
    return m_associated_contract->state()->access_control()->string_prop_query(accessor(), contract, m_id);
}

NS_END3
