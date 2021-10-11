// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_common/xproperties/xproperty_access_control.h"

#include "xbasic/xerror/xerror.h"
#include "xdata/xproperty.h"
#include "xvledger/xvledger.h"

#include <cassert>

NS_BEG3(top, contract_common, properties)


void xtop_property_utl::property_assert(bool condition, error::xerrc_t error_enum, std::string const& exception_msg) {
    if (!condition) {
        std::error_code ec{ error_enum };
        top::error::throw_error(ec, exception_msg);
    }
}

xtop_property_access_control::xtop_property_access_control(top::observer_ptr<top::base::xvbstate_t> bstate,
                                                           state_accessor::xstate_access_control_data_t ac_data,
                                                           contract_common::xcontract_execution_param_t const & param)
  : bstate_(bstate), canvas_{make_object_ptr<base::xvcanvas_t>()}, ac_data_(ac_data), m_param(param) {
    m_latest_followup_tx_hash = latest_sendtx_hash();
    m_latest_followup_tx_nonce = latest_sendtx_nonce();
}

/**
 *
 * @brief  map apis
 *
 */
template<>
void xtop_property_access_control::map_prop_create<std::string, std::string>(common::xaccount_address_t const & user,
                                                                             state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        property_assert(!property_exist(user, prop_id), "[xtop_property_access_control::map_prop_create]property already exist，prop_name: " + prop_name, error::xerrc_t::property_already_exist);
        auto prop = bstate_->new_string_map_var(prop_name, canvas_.get());
        property_assert(prop, "[xtop_property_access_control::map_prop_create]property create error，prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_create]permission denied");
    }
}

template<>
void xtop_property_access_control::map_prop_add<std::string, std::string>(common::xaccount_address_t const & user,
                                                                          state_accessor::properties::xproperty_identifier_t const & prop_id,
                                                                          std::string const & prop_key,
                                                                          std::string const & prop_value) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_add]property not exist, prop_name: " + prop_name + " , prop_key: " + prop_key + ", prop_value: " + prop_value);
        property_assert(prop->insert(prop_key, prop_value, canvas_.get()), "[xtop_property_access_control::map_prop_add]property insert error, prop_name: " + prop_name + " , prop_key: " + prop_key + ", prop_value: " + prop_value);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_add]permission denied");
    }
}

template<>
void xtop_property_access_control::map_prop_update<std::string, std::string>(common::xaccount_address_t const & user,
                                                                             state_accessor::properties::xproperty_identifier_t const & prop_id,
                                                                             std::string const & prop_key,
                                                                             std::string const & prop_value) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_update]property not exist, prop_name: " + prop_name + " , prop_key: " + prop_key + ", prop_value: " + prop_value);
        // property_assert(prop->find(prop_key), "[xtop_property_access_control::map_prop_update]property update key not exist,  prop_name: " + prop_name + " , prop_key: " + prop_key + ", prop_value: " + prop_value);

        property_assert(prop->insert(prop_key, prop_value, canvas_.get()), "[xtop_property_access_control::map_prop_update]property update key error, prop_name: " + prop_name + " , prop_key: " + prop_key + ", prop_value: " + prop_value);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_update]permission denied");
    }

}


template<>
void xtop_property_access_control::map_prop_update<std::string, std::string>(common::xaccount_address_t const & user,
                                                                             state_accessor::properties::xproperty_identifier_t const & prop_id,
                                                                             std::map<std::string, std::string> const & prop_value) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_update]property not exist, prop_name: " + prop_name);
        property_assert(prop->reset(prop_value, canvas_.get()), "[xtop_property_access_control::map_prop_update]property update error, prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_update]permission denied");
    }

}

template<>
void xtop_property_access_control::map_prop_erase<std::string, std::string>(common::xaccount_address_t const & user,
                                                                            state_accessor::properties::xproperty_identifier_t const & prop_id,
                                                                            std::string const & prop_key) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[ xtop_property_access_control::map_prop_erase]property not exist, prop_name: " + prop_name + ", prop_key: " + prop_key);
        property_assert(prop->find(prop_key), "[ xtop_property_access_control::map_prop_erase]property erase key invalid, prop_name: " + prop_name + ", prop_key: " + prop_key);

        property_assert(prop->erase(prop_key, canvas_.get()), "[ xtop_property_access_control::map_prop_erase]property erase key error, prop_name: " + prop_name + ", prop_key: " + prop_key);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_update]permission denied");
    }

}

template<>
void xtop_property_access_control::map_prop_clear<std::string, std::string>(common::xaccount_address_t const & user,
                                                                            state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_clear]property not exist, prop_name: " + prop_name);

        xassert(false); // TODO(jimmy)
        // property_assert(prop->reset(), "[xtop_property_access_control::map_prop_clear]property reset error,  prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_clear]permission denied");
    }

}

template<>
bool xtop_property_access_control::map_prop_exist<std::string, std::string>(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id, std::string const& prop_key) {
    auto prop_name = prop_id.full_name();
    if (read_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_query]property not exist, prop_name: " + prop_name + ", prop_key: " + prop_key);
        return prop->find(prop_key);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_query]permission denied");
        return false;
    }
}

template <>
std::string xtop_property_access_control::map_prop_query<std::string, std::string>(common::xaccount_address_t const & user,
                                                                                   state_accessor::properties::xproperty_identifier_t const & prop_id,
                                                                                   std::string const & prop_key) {
    auto prop_name = prop_id.full_name();
    if (read_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_query]property not exist, prop_name: " + prop_name + ", prop_key: " + prop_key);
        return prop->query(prop_key);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_query]permission denied");
        return {};
    }
}

template<>
std::map<std::string, std::string> xtop_property_access_control::map_prop_query<std::string, std::string>(common::xaccount_address_t const & user,
                                                                                                          state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_query]property not exist, prop_name: " + prop_name);
        return prop->query();
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_query]permission denied");
        return {};
    }

}

template <>
std::map<std::string, std::string> xtop_property_access_control::map_prop_query<std::string, std::string>(common::xaccount_address_t const & user,
                                                                                                          common::xaccount_address_t const & contract,
                                                                                                          state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        base::xvaccount_t _vaddr(contract.to_string());
        base::xauto_ptr<base::xvbstate_t> _bstate = base::xvchain_t::instance().get_xstatestore()->get_blkstate_store()->get_latest_connectted_block_state(_vaddr);
        if (_bstate == nullptr) {
            xwarn("[xtop_property_access_control::map_prop_query]fail-get latest connectted state.account=%s", contract.to_string().c_str());
            std::error_code ec{error::xerrc_t::state_get_failed};
            top::error::throw_error(ec, "[xtop_property_access_control::map_prop_query]state get failed");
            return {};
        }
        auto prop = bstate_->load_string_map_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::map_prop_query]property not exist, prop_name: " + prop_name);
        return prop->query();
    } else {
        std::error_code ec{error::xerrc_t::property_permission_not_allowed};
        top::error::throw_error(ec, "[xtop_property_access_control::map_prop_query]permission denied");
        return {};
    }
}

/**
 *
 * @brief str apis
 *
 */
void xtop_property_access_control::STR_PROP_CREATE(std::string const& prop_name) {
    auto prop = bstate_->new_string_var(prop_name, canvas_.get());

    property_assert(prop, "[STR_PROP_CREATE]str property create error, prop_name: " + prop_name);
}


void xtop_property_access_control::STR_PROP_UPDATE(std::string const& prop_name, std::string const& prop_value) {
    auto prop = bstate_->load_string_var(prop_name);
    property_assert(prop, "[STR_PROP_UPDATE]str property not exist, prop_name: " + prop_name + ", prop_value: " + prop_value);

    property_assert(prop->reset(prop_value, canvas_.get()), "[STR_PROP_UPDATE]str property update error, prop_name: " + prop_name + ", prop_value: " + prop_value);
}

std::string xtop_property_access_control::STR_PROP_QUERY(std::string const& prop_name) {
    auto prop = bstate_->load_string_var(prop_name);
    property_assert(prop, "[STR_PROP_QUERY]str property not exist, prop_name: " + prop_name);

    return prop->query();
}

void xtop_property_access_control::string_prop_create(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        property_assert(!property_exist(user, prop_id), "[xtop_property_access_control::string_prop_create]property already exist，prop_name: " + prop_name, error::xerrc_t::property_already_exist);
        auto prop = bstate_->new_string_var(prop_name, canvas_.get());
        property_assert(prop, "[xtop_property_access_control::string_prop_create]property create error，prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::string_prop_create]permission denied");
    }
}

void xtop_property_access_control::string_prop_update(common::xaccount_address_t const & user,
                                                      state_accessor::properties::xproperty_identifier_t const & prop_id,
                                                      std::string const & prop_value) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::string_prop_update]property not exist, prop_name: " + prop_name);
        property_assert(prop->reset(prop_value, canvas_.get()), "[xtop_property_access_control::string_prop_update]property update error, prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::string_prop_update]permission denied");
    }

}

void xtop_property_access_control::string_prop_clear(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::string_prop_clear]property not exist, prop_name: " + prop_name);

        xassert(false); // TODO(jimmy)
        // property_assert(prop->reset(), "[xtop_property_access_control::string_prop_clear]property reset error,  prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::string_prop_clear]permission denied");
    }

}

std::string xtop_property_access_control::string_prop_query(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        auto prop = bstate_->load_string_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::string_prop_query]property not exist, prop_name: " + prop_name);
        return prop->query();
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::string_prop_query]permission denied");
        return {};
    }

}

std::string xtop_property_access_control::string_prop_query(common::xaccount_address_t const & user,
                                                            common::xaccount_address_t const & contract,
                                                            state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        base::xvaccount_t _vaddr(contract.to_string());
        base::xauto_ptr<base::xvbstate_t> _bstate = base::xvchain_t::instance().get_xstatestore()->get_blkstate_store()->get_latest_connectted_block_state(_vaddr);
        if (_bstate == nullptr) {
            xwarn("[xtop_property_access_control::string_prop_query]fail-get latest connectted state.account=%s", contract.to_string().c_str());
            std::error_code ec{error::xerrc_t::state_get_failed};
            top::error::throw_error(ec, "[xtop_property_access_control::string_prop_query]state get failed");
            return {};
        }
        auto prop = bstate_->load_string_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::string_prop_query]property not exist, prop_name: " + prop_name);
        return prop->query();
    } else {
        std::error_code ec{error::xerrc_t::property_permission_not_allowed};
        top::error::throw_error(ec, "[xtop_property_access_control::string_prop_query]permission denied");
        return {};
    }
}

/**
 *
 * @brief token apis
 *
 */
void xtop_property_access_control::token_prop_create(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        property_assert(!property_exist(user, prop_id), "[xtop_property_access_control::token_prop_create]property already exist, prop_name: " + prop_name, error::xerrc_t::property_already_exist);
        auto prop = bstate_->new_token_var(prop_name, canvas_.get());

        property_assert(prop, "[xtop_property_access_control::token_prop_create]property create error, prop_name: " + prop_name);
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::token_prop_create]permission denied");
    }

}

uint64_t xtop_property_access_control::withdraw(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id, uint64_t amount) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        if (!bstate_->find_property(data::XPROPERTY_BALANCE_AVAILABLE)) {
            bstate_->new_token_var(data::XPROPERTY_BALANCE_AVAILABLE, canvas_.get());
        }
        auto prop = bstate_->load_token_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::withdraw]property not exist, token_prop: " + prop_name + ", amount: " + std::to_string(amount));
        return prop->withdraw((base::vtoken_t)amount, canvas_.get());
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::withdraw]permission denied");
        return {};
    }
}

uint64_t xtop_property_access_control::deposit(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id, uint64_t amount) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        if (!bstate_->find_property(data::XPROPERTY_BALANCE_AVAILABLE)) {
            bstate_->new_token_var(data::XPROPERTY_BALANCE_AVAILABLE, canvas_.get());
        }
        auto prop = bstate_->load_token_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::deposit]token property not exist, token_prop: " + prop_name + ", amount: " + std::to_string(amount));
        return prop->deposit((base::vtoken_t)amount, canvas_.get());
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::deposit]permission denied");
        return {};
    }

}

uint64_t xtop_property_access_control::balance(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & prop_id) {
    auto prop_name = prop_id.full_name();
    if (write_permitted(user, prop_id)) {
        if (!bstate_->find_property(data::XPROPERTY_BALANCE_AVAILABLE)) {
            bstate_->new_token_var(data::XPROPERTY_BALANCE_AVAILABLE, canvas_.get());
        }
        auto prop = bstate_->load_token_var(prop_name);
        property_assert(prop, "[xtop_property_access_control::balance]property not exist, token_prop: " + prop_name);
        return (uint64_t)prop->get_balance();
    } else {
        std::error_code ec{ error::xerrc_t::property_permission_not_allowed };
        top::error::throw_error(ec, "[xtop_property_access_control::balance]permission denied");
        return {};
    }

}


std::string xtop_property_access_control::src_code(state_accessor::properties::xproperty_identifier_t const & prop_id, std::error_code & ec) const {
    assert(!ec);
    auto prop_name = prop_id.full_name();
    auto prop = bstate_->load_code_var(prop_name);
    property_assert(prop, "[xtop_property_access_control::src_code]property not exist, prop_name: " + prop_name);

    return prop->query();
}

std::string xtop_property_access_control::src_code(state_accessor::properties::xproperty_identifier_t const & prop_id) const {
    std::error_code ec;
    auto r = src_code(prop_id, ec);
    top::error::throw_error(ec);
    return r;
}

void xtop_property_access_control::deploy_src_code(state_accessor::properties::xproperty_identifier_t const & prop_id, std::string src_code, std::error_code & ec) {
    assert(!ec);
    if (bstate_->find_property(prop_id.full_name())) {
        ec = error::xerrc_t::property_already_exist;
        return;
    }

    auto src_prop = bstate_->new_code_var(prop_id.full_name(), canvas_.get());
    if (!src_prop->deploy_code(src_code, canvas_.get())) {
        ec = error::xerrc_t::deploy_code_failed;
    }
}

void xtop_property_access_control::deploy_src_code(state_accessor::properties::xproperty_identifier_t const & prop_id, std::string src_code) {
    std::error_code ec;
    deploy_src_code(prop_id, std::move(src_code), ec);
    top::error::throw_error(ec);
}

xbyte_buffer_t xtop_property_access_control::bin_code(state_accessor::properties::xproperty_identifier_t const & prop_id, std::error_code & ec) const {
    assert(!ec);
    auto prop_name = prop_id.full_name();
    auto prop = bstate_->load_code_var(prop_name);
    property_assert(prop, "[xtop_property_access_control::src_code]property not exist, prop_name: " + prop_name);

    return { std::begin(prop->query()), std::end(prop->query()) };
}

xbyte_buffer_t xtop_property_access_control::bin_code(state_accessor::properties::xproperty_identifier_t const & prop_id) const {
    std::error_code ec;
    auto r = bin_code(prop_id, ec);
    top::error::throw_error(ec);
    return r;
}

void xtop_property_access_control::deploy_bin_code(state_accessor::properties::xproperty_identifier_t const & prop_id, xbyte_buffer_t bin_code, std::error_code & ec) {
    assert(!ec);
    if (bstate_->find_property(prop_id.full_name())) {
        ec = error::xerrc_t::property_already_exist;
        return;
    }

    auto src_prop = bstate_->new_code_var(prop_id.full_name(), canvas_.get());
    if (!src_prop->deploy_code({ std::begin(bin_code), std::end(bin_code) }, canvas_.get())) {
        ec = error::xerrc_t::deploy_code_failed;
    }
}

void xtop_property_access_control::deploy_bin_code(state_accessor::properties::xproperty_identifier_t const & prop_id, xbyte_buffer_t bin_code) {
    std::error_code ec;
    deploy_bin_code(prop_id, std::move(bin_code), ec);
    top::error::throw_error(ec);
}

/**
 *
 * @brief  context apis
 *
 */
common::xaccount_address_t xtop_property_access_control::address() const {
    return common::xaccount_address_t{bstate_->get_address()};
}

uint64_t xtop_property_access_control::blockchain_height() const {
    return bstate_->get_block_height();
}

common::xlogic_time_t xtop_property_access_control::time() const {
    return m_param.m_clock;
}

common::xlogic_time_t xtop_property_access_control::timestamp() const {
    return m_param.m_timestamp;
}

void xtop_property_access_control::create_time(std::error_code& ec) {
    assert(!ec);
    assert(bstate_ != nullptr);

    if (false == bstate_->find_property(data::XPROPERTY_ACCOUNT_CREATE_TIME)) {
        auto propobj = bstate_->new_uint64_var(data::XPROPERTY_ACCOUNT_CREATE_TIME, canvas_.get());
        uint64_t create_time = m_param.m_clock == 0 ? base::TOP_BEGIN_GMTIME : m_param.m_clock;
        propobj->set(create_time, canvas_.get());
    }

    return;
}

void xtop_property_access_control::create_time() {
    std::error_code ec;
    create_time(ec);
    top::error::throw_error(ec);
    return;
}

bool xtop_property_access_control::property_exist(common::xaccount_address_t const & user,
                                                  state_accessor::properties::xproperty_identifier_t const & property_id,
                                                  std::error_code & ec) const {
    assert(!ec);
    auto prop_name = property_id.full_name();
    if (read_permitted(user, property_id)) {
        return bstate_->find_property(prop_name);
    } else {
        ec = error::xerrc_t::property_permission_not_allowed;
        return false;
    }
}

bool xtop_property_access_control::property_exist(common::xaccount_address_t const & user, state_accessor::properties::xproperty_identifier_t const & property_id) const {
    std::error_code ec;
    auto ret = property_exist(user, property_id, ec);
    top::error::throw_error(ec);
    return ret;
}

bool xtop_property_access_control::system_property(state_accessor::properties::xproperty_identifier_t const & property_id) const {
    return false;
}

/**
 *
 * @brief util apis
 *
 */
void xtop_property_access_control::property_assert(bool condition, std::string const& exception_msg, error::xerrc_t error_enum) const {
    xtop_property_utl::property_assert(condition, error_enum, exception_msg);
}

void xtop_property_access_control::load_access_control_data(std::string const & json) {

}
void xtop_property_access_control::load_access_control_data(state_accessor::xstate_access_control_data_t const & data) {
    ac_data_ = data;
}

uint256_t xtop_property_access_control::latest_sendtx_hash(std::error_code & ec) const {
    assert(!ec);

    uint256_t hash;
    assert(bstate_ != nullptr);
    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        // base::xvpropertyrules_t::is_valid_native_property(key)) {
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
    }

    auto value = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO)->query(data::XPROPERTY_TX_INFO_LATEST_SENDTX_HASH);
    if (value.empty())
        return hash;

    return uint256_t((uint8_t *)value.c_str());
}

uint256_t xtop_property_access_control::latest_sendtx_hash() const {
    std::error_code ec;
    auto res = latest_sendtx_hash(ec);
    top::error::throw_error(ec);
    return res;
}

void xtop_property_access_control::latest_sendtx_hash(uint256_t hash, std::error_code & ec) {
    assert(!ec);
    assert(bstate_ != nullptr);

    std::string hash_str(reinterpret_cast<char *>(hash.data()), hash.size());

    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        // base::xvpropertyrules_t::is_valid_native_property(key)) {
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
    }
    auto propobj = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO);
    xassert(propobj != nullptr);
    if (propobj->find(data::XPROPERTY_TX_INFO_LATEST_SENDTX_HASH)) {
        auto old_value = propobj->query(data::XPROPERTY_TX_INFO_LATEST_SENDTX_HASH);
        if (old_value == hash_str) {
            return;
        }
    }
    propobj->insert(data::XPROPERTY_TX_INFO, data::XPROPERTY_TX_INFO_LATEST_SENDTX_HASH, canvas_.get());
}

void xtop_property_access_control::latest_sendtx_hash(uint256_t hash) {
    std::error_code ec;
    latest_sendtx_hash(hash, ec);
    top::error::throw_error(ec);
}

uint64_t xtop_property_access_control::latest_sendtx_nonce(std::error_code & ec) const {
    assert(!ec);

    uint64_t nonce{0};
    assert(bstate_ != nullptr);
    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
        return nonce;
    }

    auto value = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO)->query(data::XPROPERTY_TX_INFO_LATEST_SENDTX_NUM);
    if (value.empty())
        return nonce;

    return base::xstring_utl::touint64(value);
}

uint64_t xtop_property_access_control::latest_sendtx_nonce() const {
    std::error_code ec;
    auto res = latest_sendtx_nonce(ec);
    top::error::throw_error(ec);
    return res;
}

void xtop_property_access_control::latest_sendtx_nonce(uint64_t nonce, std::error_code & ec) {
    assert(!ec);
    assert(bstate_ != nullptr);

    std::string nonce_str = base::xstring_utl::tostring(nonce);

    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        // base::xvpropertyrules_t::is_valid_native_property(key)) {
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
    }
    auto propobj = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO);
    xassert(propobj != nullptr);
    if (propobj->find(data::XPROPERTY_TX_INFO_LATEST_SENDTX_NUM)) {
        auto old_value = propobj->query(data::XPROPERTY_TX_INFO_LATEST_SENDTX_NUM);
        if (old_value == nonce_str) {
            return;
        }
    }
    propobj->insert(data::XPROPERTY_TX_INFO_LATEST_SENDTX_NUM, nonce_str, canvas_.get());
}

void xtop_property_access_control::latest_sendtx_nonce(uint64_t nonce) {
    std::error_code ec;
    latest_sendtx_nonce(nonce, ec);
    top::error::throw_error(ec);
}

uint256_t xtop_property_access_control::latest_followup_tx_hash(std::error_code & ec) const {
    return m_latest_followup_tx_hash;
}

uint256_t xtop_property_access_control::latest_followup_tx_hash() const {
    std::error_code ec;
    auto res = latest_followup_tx_hash(ec);
    top::error::throw_error(ec);
    return res;
}

void xtop_property_access_control::latest_followup_tx_hash(uint256_t hash, std::error_code & ec) {
    m_latest_followup_tx_hash = hash;
}

void xtop_property_access_control::latest_followup_tx_hash(uint256_t hash) {
    std::error_code ec;
    latest_followup_tx_hash(hash, ec);
    top::error::throw_error(ec);
}

uint64_t xtop_property_access_control::latest_followup_tx_nonce(std::error_code & ec) const {
    return m_latest_followup_tx_nonce;
}

uint64_t xtop_property_access_control::latest_followup_tx_nonce() const {
    std::error_code ec;
    auto res = latest_followup_tx_nonce(ec);
    top::error::throw_error(ec);
    return res;
}

void xtop_property_access_control::latest_followup_tx_nonce(uint64_t nonce, std::error_code & ec) {
    m_latest_followup_tx_nonce = nonce;
}

void xtop_property_access_control::latest_followup_tx_nonce(uint64_t nonce) {
    std::error_code ec;
    latest_followup_tx_nonce(nonce, ec);
    top::error::throw_error(ec);
}

uint64_t xtop_property_access_control::recvtx_num(std::error_code & ec) const {
    assert(!ec);

    uint64_t num{0};
    assert(bstate_ != nullptr);
    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
        return num;
    }

    auto value = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO)->query(data::XPROPERTY_TX_INFO_RECVTX_NUM);
    if (value.empty())
        return num;

    return base::xstring_utl::touint64(value);
}

uint64_t xtop_property_access_control::recvtx_num() const {
    std::error_code ec;
    auto res = recvtx_num(ec);
    top::error::throw_error(ec);
    return res;
}

void xtop_property_access_control::recvtx_num(uint64_t num, std::error_code & ec) {
    assert(!ec);
    assert(bstate_ != nullptr);

    std::string nonce_str = base::xstring_utl::tostring(num);

    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        // base::xvpropertyrules_t::is_valid_native_property(key)) {
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
    }
    auto propobj = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO);
    xassert(propobj != nullptr);
    if (propobj->find(data::XPROPERTY_TX_INFO_RECVTX_NUM)) {
        auto old_value = propobj->query(data::XPROPERTY_TX_INFO_RECVTX_NUM);
        if (old_value == nonce_str) {
            return;
        }
    }
    propobj->insert(data::XPROPERTY_TX_INFO_RECVTX_NUM, nonce_str, canvas_.get());
}

void xtop_property_access_control::recvtx_num(uint64_t num) {
    std::error_code ec;
    recvtx_num(num, ec);
    top::error::throw_error(ec);
}

uint64_t xtop_property_access_control::unconfirm_sendtx_num(std::error_code & ec) const {
    assert(!ec);

    uint64_t num{0};
    assert(bstate_ != nullptr);
    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
        return num;
    }

    auto value = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO)->query(data::XPROPERTY_TX_INFO_UNCONFIRM_TX_NUM);
    if (value.empty())
        return num;

    return base::xstring_utl::touint64(value);
}

uint64_t xtop_property_access_control::unconfirm_sendtx_num() const {
    std::error_code ec;
    auto res = unconfirm_sendtx_num(ec);
    top::error::throw_error(ec);
    return res;
}

void xtop_property_access_control::unconfirm_sendtx_num(uint64_t num, std::error_code & ec) {
    assert(!ec);
    assert(bstate_ != nullptr);

    std::string nonce_str = base::xstring_utl::tostring(num);

    if (!bstate_->find_property(data::XPROPERTY_TX_INFO)) {
        // ec = error::xerrc_t::property_not_exist;
        // base::xvpropertyrules_t::is_valid_native_property(key)) {
        bstate_->new_string_map_var(data::XPROPERTY_TX_INFO, canvas_.get());
    }
    auto propobj = bstate_->load_string_map_var(data::XPROPERTY_TX_INFO);
    xassert(propobj != nullptr);
    if (propobj->find(data::XPROPERTY_TX_INFO_UNCONFIRM_TX_NUM)) {
        auto old_value = propobj->query(data::XPROPERTY_TX_INFO_UNCONFIRM_TX_NUM);
        if (old_value == nonce_str) {
            return;
        }
    }
    propobj->insert(data::XPROPERTY_TX_INFO_UNCONFIRM_TX_NUM, nonce_str, canvas_.get());
}

void xtop_property_access_control::unconfirm_sendtx_num(uint64_t num) {
    std::error_code ec;
    unconfirm_sendtx_num(num, ec);
    top::error::throw_error(ec);
}

std::string xtop_property_access_control::binlog(std::error_code & ec) const {
    std::string r;
    assert(canvas_ != nullptr);
    if (canvas_->encode(r) < 0) {
        ec = error::xerrc_t::get_binlog_failed;
    }

    return r;
}

std::string xtop_property_access_control::binlog() const {
    std::error_code ec;
    auto r = binlog(ec);
    top::error::throw_error(ec);
    return r;
}

std::string xtop_property_access_control::fullstate_bin() const {
    std::string fullstate_bin;
    bstate_->take_snapshot(fullstate_bin);

    return fullstate_bin;
}

bool xtop_property_access_control::read_permitted(common::xaccount_address_t const & reader,
                                                  state_accessor::properties::xproperty_identifier_t const & property_id) const noexcept {
    return true;
}

bool xtop_property_access_control::read_permitted(common::xaccount_address_t const & reader, std::string const & property_full_name) const noexcept {
    return true;
}

bool xtop_property_access_control::write_permitted(common::xaccount_address_t const & writer,
                                                   state_accessor::properties::xproperty_identifier_t const & property_id) const noexcept {
    return true;
}

NS_END3
