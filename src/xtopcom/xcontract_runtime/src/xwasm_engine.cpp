// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xbase/xbase.h"
#include "xcontract_common/xcontract_state.h"
#include "xcontract_runtime/xerror/xerror.h"
#include "xcontract_runtime/xuser/xwasm/xwasm_engine.h"

extern "C" bool validate_wasm_with_content(uint8_t *s, uint32_t size);

NS_BEG3(top, contract_runtime, user)

void xtop_wasm_engine::deploy_contract(xbyte_buffer_t const& code, observer_ptr<contract_common::xcontract_execution_context_t> exe_ctx) {
    if (!validate_wasm_with_content((uint8_t*)code.data(), code.size())) {
        std::error_code ec{ error::xenum_errc::enum_wasm_code_invalid };
        top::error::throw_error(ec, "invalid wasm code");
    }
    exe_ctx->contract_state()->deploy_src_code(std::string{(char*)code.data(), code.size()});

}

void xtop_wasm_engine::call_contract(std::string const& func_name, std::vector<xbyte_buffer_t> const& params, observer_ptr<contract_common::xcontract_execution_context_t> exe_ctx) {
}

void xtop_wasm_engine::deploy_contract_erc20(std::vector<xbyte_buffer_t> const& params, observer_ptr<contract_common::xcontract_execution_context_t> exe_ctx) {
    if (!validate_wasm_with_content((uint8_t*)params[0].data(), params[0].size())) {
        std::error_code ec{ error::xenum_errc::enum_wasm_code_invalid };
        top::error::throw_error(ec, "invalid wasm code");
    }

    auto contract_state = exe_ctx->contract_state();
    auto state_account = contract_state->state_account_address();
    contract_common::properties::xproperty_identifier_t src_property_id{"src_code", contract_common::properties::xproperty_type_t::src_code, contract_common::properties::xproperty_category_t::user};
    contract_state->access_control()->code_prop_create(state_account, src_property_id);
    contract_state->access_control()->code_prop_update(state_account, src_property_id, std::string{params[0].data(), params[0].data() + params[0].size()});
    contract_common::properties::xproperty_identifier_t balances_property_id{"map_balances", contract_common::properties::xproperty_type_t::map, contract_common::properties::xproperty_category_t::user};
    contract_state->access_control()->map_prop_create<std::string, std::string>(state_account, balances_property_id);
    assert(params.size() == 3); // 1: erc20 symbal, 2: total supply [0: the code]

    // contract_common::properties::xproperty_identifier_t balances_property_id{std::string{params[0].data(), params[0].size()}, contract_common::properties::xproperty_type_t::map, contract_common::properties::xproperty_category_t::user};
    contract_state->access_control()->STR_PROP_CREATE(std::string{params[0].data(), params[0].data() + params[0].size()});
    contract_state->access_control()->map_prop_add<std::string, std::string>(state_account, src_property_id, state_account.value(), std::string{params[1].data(), params[1].data() + params[1].size()});
}


void xtop_wasm_engine::call_contract_erc20(std::string const& func_name, std::vector<xbyte_buffer_t> const& params, observer_ptr<contract_common::xcontract_execution_context_t> exe_ctx) {
    if (func_name == "balanceof") {
        contract_common::properties::xproperty_identifier_t balances_property_id{"map_balances", contract_common::properties::xproperty_type_t::map, contract_common::properties::xproperty_category_t::user};
        auto contract_state = exe_ctx->contract_state();
        auto state_account = contract_state->state_account_address();
        contract_state->access_control()->map_prop_query<std::string, std::string>(state_account, balances_property_id, state_account.value());
    }
}

NS_END3