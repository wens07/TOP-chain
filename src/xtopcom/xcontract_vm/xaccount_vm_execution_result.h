// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xcontract_runtime/xtransaction_execution_result.h"
#include "xcontract_vm/xerror/xerror.h"

#include <string>
#include <system_error>
#include <vector>

NS_BEG2(top, contract_vm)

struct xtop_account_vm_execution_status {
    std::error_code ec{error::xerrc_t::ok};
    std::string extra_msg{};
};
using xaccount_vm_execution_status_t = xtop_account_vm_execution_status;

struct xtop_account_vm_execution_result {
    xaccount_vm_execution_status_t status;
    std::vector<contract_runtime::xtransaction_execution_result_t> transaction_results;
    std::string binlog;
    std::string contract_state_snapshot;
};
using xaccount_vm_execution_result_t = xtop_account_vm_execution_result;

NS_END2
