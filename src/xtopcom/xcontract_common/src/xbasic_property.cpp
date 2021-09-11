// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_common/xproperties/xbasic_property.h"

#include "xbasic/xutility.h"
#include "xcontract_common/xbasic_contract.h"

NS_BEG3(top, contract_common, properties)

static state_accessor::properties::xproperty_category_t lookup_property_category(std::string const & name,
                                                                                 state_accessor::properties::xproperty_type_t const type) {
    static std::unordered_map<std::string, std::unordered_map<state_accessor::properties::xproperty_type_t, state_accessor::properties::xproperty_category_t>> const dict{
        {"balance", {{state_accessor::properties::xproperty_type_t::token, state_accessor::properties::xproperty_category_t::system}}},
        {"nonce", {{state_accessor::properties::xproperty_type_t::nonce, state_accessor::properties::xproperty_category_t::system}}}
    };

    state_accessor::properties::xproperty_category_t property_category{state_accessor::properties::xproperty_category_t::user};
    do {
        auto const it = dict.find(name);
        if (it == std::end(dict)) {
            break;
        }

        auto const & inner_dict = top::get<std::unordered_map<state_accessor::properties::xproperty_type_t, state_accessor::properties::xproperty_category_t>>(*it);
        auto const inner_it = inner_dict.find(type);
        if (inner_it == std::end(inner_dict)) {
            break;
        }

        property_category = top::get<state_accessor::properties::xproperty_category_t>(*inner_it);
    } while (false);

    return property_category;
}

xtop_basic_property::xtop_basic_property(std::string const & name, state_accessor::properties::xproperty_type_t type, observer_ptr<xbasic_contract_t> associated_contract) noexcept
    : m_associated_contract{ associated_contract }
    , m_id{ name, type, lookup_property_category(name, type) }
    , m_owner{ associated_contract->address() } {
}

void xtop_basic_property::create() {
}

state_accessor::properties::xproperty_identifier_t const & xtop_basic_property::identifier() const {
    return m_id;
}

common::xaccount_address_t xtop_basic_property::owner() const {
    return m_owner;
}

common::xaccount_address_t xtop_basic_property::accessor() const {
    return m_associated_contract->state()->state_account_address();
}

NS_END3
