// Copyright (c) 2018-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbase/xobject_ptr.h"
#include "xbasic/xmemory.hpp"
#include "xcontract_runtime/xuser/xuser_action_runtime.h"
#include "xcontract_vm/xaccount_vm_execution_result.h"
#include "xcontract_vm/xvm_executor_face.h"
#include "xdata/xcons_transaction.h"
#include "xstate_accessor/xstate_accessor.h"
#include "xsystem_contract_runtime/xsystem_action_runtime.h"
#include "xsystem_contract_runtime/xsystem_contract_manager.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push, 0)
#endif

#include "xvledger/xvstate.h"

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

NS_BEG2(top, contract_vm)

struct xtop_account_vm_execution_assemble {
    std::vector<data::xcons_transaction_ptr_t> txs;
    xobject_ptr_t<base::xvbstate_t> block_state;
    data::xblock_consensus_para_t cs_para;
};
using xaccount_vm_execution_assemble_t = xtop_account_vm_execution_assemble;

class xtop_account_vm : public xvm_executor_face_t {
private:
    std::unique_ptr<contract_runtime::user::xuser_action_runtime_t> user_action_runtime_{top::make_unique<contract_runtime::user::xuser_action_runtime_t>()};
    std::unique_ptr<contract_runtime::system::xsystem_action_runtime_t> sys_action_runtime_;

public:
    // xtop_account_vm() = default;
    xtop_account_vm(xtop_account_vm const &) = delete;
    xtop_account_vm & operator=(xtop_account_vm const &) = delete;
    xtop_account_vm(xtop_account_vm &&) = default;
    xtop_account_vm & operator=(xtop_account_vm &&) = default;
    ~xtop_account_vm() override = default;

    explicit xtop_account_vm(observer_ptr<contract_runtime::system::xsystem_contract_manager_t> const & system_contract_manager);

    xaccount_vm_output_t execute(std::vector<data::xcons_transaction_ptr_t> const & txs,
                                 observer_ptr<base::xvbstate_t> const & state_pack,
                                 data::xblock_consensus_para_t const & cs_para);

private:
    contract_runtime::xtransaction_execution_result_t execute_action(std::unique_ptr<data::xbasic_top_action_t const> action,
                                                                     contract_common::xcontract_execution_param_t const & param,
                                                                     state_accessor::xstate_accessor_t & ac);
    xaccount_vm_output_t pack(std::vector<data::xcons_transaction_ptr_t> const & txs,
                              xaccount_vm_execution_result_t const & result,
                              contract_common::xcontract_execution_param_t const & param,
                              state_accessor::xstate_accessor_t & ac);
    void preprocess(std::vector<data::xcons_transaction_ptr_t> const & txs, state_accessor::xstate_accessor_t & sa, xaccount_vm_execution_result_t & r);
    void abort(const size_t start_index, const size_t size, xaccount_vm_execution_result_t & result);
};
using xaccount_vm_t = xtop_account_vm;

NS_END2
