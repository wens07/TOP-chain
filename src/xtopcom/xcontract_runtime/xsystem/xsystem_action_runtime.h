// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xcontract_runtime/xaction_runtime.h"
#include "xdata/xconsensus_action.h"

NS_BEG2(top, contract_runtime)

template <>
class xtop_action_runtime<data::xconsensus_action_t<data::xtop_action_type_t::system>> {
public:
    xtop_action_runtime() = default;
    xtop_action_runtime(xtop_action_runtime const &) = delete;
    xtop_action_runtime & operator=(xtop_action_runtime const &) = delete;
    xtop_action_runtime(xtop_action_runtime &&) = default;
    xtop_action_runtime & operator=(xtop_action_runtime &&) = default;
    ~xtop_action_runtime() = default;

    std::unique_ptr<xaction_session_t<data::xconsensus_action_t<data::xtop_action_type_t::system>>> new_session(observer_ptr<contract_common::xcontract_state_t> contract_state);

    xtransaction_execution_result_t execute(observer_ptr<contract_common::xcontract_execution_context_t> tx_ctx);
};

NS_END2

NS_BEG3(top, contract_runtime, system)
using xsystem_action_runtime_t = xtop_action_runtime<data::xconsensus_action_t<data::xtop_action_type_t::system>>;
NS_END3