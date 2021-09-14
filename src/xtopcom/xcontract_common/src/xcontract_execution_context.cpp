// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_common/xcontract_execution_context.h"

#include "xbasic/xutility.h"
#include "xcontract_common/xerror/xerror.h"

NS_BEG2(top, contract_common)

xtop_contract_execution_context::xtop_contract_execution_context(xobject_ptr_t<data::xtransaction_t> tx, observer_ptr<xcontract_state_t> s) noexcept
  : m_contract_state{s}, m_tx{std::move(tx)} {
}

xtop_contract_execution_context::xtop_contract_execution_context(data::xbasic_top_action_t const & action, observer_ptr<xcontract_state_t> s, xcontract_execution_param_t param) noexcept
  : m_contract_state{s},  m_action{&action}, m_param(param) {
}

observer_ptr<xcontract_state_t> xtop_contract_execution_context::contract_state() const noexcept {
    return m_contract_state;
}

xcontract_execution_stage_t xtop_contract_execution_context::execution_stage() const noexcept {
    return m_execution_stage;
}

void xtop_contract_execution_context::execution_stage(xcontract_execution_stage_t const stage) noexcept {
    m_execution_stage = stage;
}

data::xconsensus_action_stage_t xtop_contract_execution_context::consensus_action_stage() const noexcept {
    return m_stage;
}

void xtop_contract_execution_context::consensus_action_stage(data::xconsensus_action_stage_t const stage) noexcept {
    m_stage = stage;
}

xcontract_execution_result_t xtop_contract_execution_context::execution_result() const noexcept {
    return m_execution_result;
}

void xtop_contract_execution_context::add_followup_transaction(data::xtransaction_ptr_t && tx, xfollowup_transaction_schedule_type_t type) {
    m_execution_result.output.followup_transaction_data.emplace_back(std::move(tx), type);
}

void xtop_contract_execution_context::receipt_data(std::map<std::string, xbyte_buffer_t> receipt_data) {
    m_receipt_data = std::move(receipt_data);
}

std::map<std::string, xbyte_buffer_t> & xtop_contract_execution_context::receipt_data() noexcept {
    return m_execution_result.output.receipt_data;
}

xbyte_buffer_t const & xtop_contract_execution_context::receipt_data(std::string const & key, std::error_code & ec) const noexcept {
    static xbyte_buffer_t const empty;
    auto const it = m_receipt_data.find(key);
    if (it != std::end(m_receipt_data)) {
        return top::get<xbyte_buffer_t>(*it);
    }

    ec = error::xerrc_t::receipt_data_not_found;
    return empty;
}

common::xaccount_address_t xtop_contract_execution_context::sender() const {
    return common::xaccount_address_t{m_tx->get_source_addr()};
}

common::xaccount_address_t xtop_contract_execution_context::recver() const {
    if (m_tx) return common::xaccount_address_t{m_tx->get_target_addr()};
    common::xaccount_address_t ret;
    switch (m_action->type()) {
        case data::xtop_action_type_t::system:{
            ret = common::xaccount_address_t{dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::system> const *>(m_action)->to_address()};
            break;
        }
        case data::xtop_action_type_t::user:{
            ret = common::xaccount_address_t{dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::user> const *>(m_action)->to_address()};
            break;
        }
        default:
            break;
    }
    return ret;
}

common::xaccount_address_t xtop_contract_execution_context::contract_address() const {
    return recver();
}

data::enum_xtransaction_type xtop_contract_execution_context::transaction_type() const noexcept {
    if (m_tx) return static_cast<data::enum_xtransaction_type>(m_tx->get_tx_type());
    data::enum_xtransaction_type ret = data::enum_xtransaction_type::xtransaction_type_max;
    switch (m_action->type()) {
        case data::xtop_action_type_t::system:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::system> const *>(m_action)->transaction_type();
            break;
        }
        case data::xtop_action_type_t::user:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::user> const *>(m_action)->transaction_type();
            break;
        }
        default:
            break;
    }
    return ret;
}

std::string xtop_contract_execution_context::action_name() const {
    if (m_tx) return m_tx->get_target_action_name();
    std::string ret;
    switch (m_action->type()) {
        case data::xtop_action_type_t::system:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::system> const *>(m_action)->action_name();
            break;
        }
        case data::xtop_action_type_t::user:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::user> const *>(m_action)->action_name();
            break;
        }
        default:
            break;
    }
    return ret;
}

data::enum_xaction_type xtop_contract_execution_context::action_type() const {
    return m_tx->get_target_action().get_action_type();
}

xbyte_buffer_t xtop_contract_execution_context::action_data() const {
    if (m_tx) {
        auto const & action_param = m_tx->get_target_action().get_action_param();
        return {std::begin(action_param), std::end(action_param)};
    }

    xbyte_buffer_t ret;
    switch (m_action->type()) {
        case data::xtop_action_type_t::system:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::system> const *>(m_action)->action_data();
            break;
        }
        case data::xtop_action_type_t::user:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::user> const *>(m_action)->action_data();
            break;
        }
        default:
            break;
    }
    return ret;
}

data::xconsensus_action_stage_t xtop_contract_execution_context::action_stage() const {
    data::xconsensus_action_stage_t ret = data::xconsensus_action_stage_t::invalid;
    switch (m_action->type()) {
        case data::xtop_action_type_t::system:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::system> const *>(m_action)->stage();
            break;
        }
        case data::xtop_action_type_t::user:{
            ret = dynamic_cast<data::xconsensus_action_t<data::xtop_action_type_t::user> const *>(m_action)->stage();
            break;
        }
        default:
            break;
    }
    return ret;
}

common::xlogic_time_t xtop_contract_execution_context::time() const {
    return m_param.m_clock;
}

NS_END2
