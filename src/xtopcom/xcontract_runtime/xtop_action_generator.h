// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbasic/xmemory.hpp"
#include "xdata/xcons_transaction.h"
#include "xdata/xtop_action.h"

#include <memory>
#include <vector>

NS_BEG2(top, contract_runtime)

class xtop_action_generator {
public:
    static data::xbasic_top_action_t generate(xobject_ptr_t<data::xcons_transaction_t> const & tx);
    static std::vector<data::xbasic_top_action_t> generate(std::vector<xobject_ptr_t<data::xcons_transaction_t>> const & txs);
};
using xaction_generator_t = xtop_action_generator;

NS_END2
