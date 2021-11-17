
#define private public
#define protected public
#include "tests/mock/xvchain_creator.hpp"
#include "xbase/xobject_ptr.h"
#include "xblockstore/xblockstore_face.h"
#include "xcontract_common/xcontract_execution_result.h"
#include "xcontract_common/xcontract_state.h"
#include "xcontract_common/xproperties/xproperty_access_control.h"
#include "xcontract_runtime/xtop_action_generator.h"
#include "xcontract_vm/xaccount_vm.h"
#include "xdata/xblocktool.h"
#include "xdata/xdata_common.h"
#include "xdata/xtransaction_v2.h"
#include "xdb/xdb_face.h"
#include "xdb/xdb_factory.h"
#include "xstore/xstore_face.h"
#include "xsystem_contract_runtime/xsystem_action_runtime.h"
#include "xsystem_contract_runtime/xsystem_contract_manager.h"
#include "xsystem_contracts/xsystem_contract_addresses.h"
#include "xsystem_contracts/xtransfer_contract.h"
#include "xvledger/xvledger.h"
#include "xvledger/xvstate.h"
#include "xvm/xsystem_contracts/xelection/xrec/xrec_standby_pool_contract_new.h"
#include "xvm/xsystem_contracts/xregistration/xrec_registration_contract_new.h"

#include <gtest/gtest.h>

#include <memory>

NS_BEG3(top, tests, contract_framework_api)

using namespace top::contract_common;
using namespace top::contract_runtime;
std::string const contract_address = system_contracts::transfer_address;


class test_system_contract : public contract_common::xtop_basic_contract {
public:
    void contract_execution_context(observer_ptr<xcontract_execution_context_t> const& exec_context);
    xcontract_execution_result_t execute(observer_ptr<xcontract_execution_context_t> exec_ctx) override final;
};


class test_contract_framework_api: public testing::Test {
public:
    test_system_contract contract_handle;

protected:
    void SetUp() override;
    void TearDown() override;
};

void test_system_contract::contract_execution_context(observer_ptr<xcontract_execution_context_t> const& exec_context) {
    m_associated_execution_context = exec_context;
}


xcontract_execution_result_t test_system_contract::execute(observer_ptr<xcontract_execution_context_t> exec_ctx) {
    return xcontract_execution_result_t{};
}

void test_contract_framework_api::SetUp() {}
void test_contract_framework_api::TearDown(){}


TEST_F(test_contract_framework_api, test_asset_out) {
    uint64_t token_amount = 10000000;
    data::xproperty_asset asset_out{data::XPROPERTY_ASSET_TOP, token_amount};
    base::xstream_t stream(base::xcontext_t::instance());
    stream << asset_out.m_token_name;
    stream << asset_out.m_amount;
    auto src_asset_data = std::string{(char*)stream.data(), stream.size()};

    std::map<std::string, xbyte_buffer_t> for_receipt_data;
    for_receipt_data[contract_common::RECEITP_DATA_ASSET_OUT] = xbyte_buffer_t{src_asset_data.begin(), src_asset_data.end()};


    observer_ptr<xcontract_execution_context_t> ctx{new xcontract_execution_context_t};
    ctx->receipt_data(for_receipt_data);
    contract_handle.contract_execution_context(ctx);

    std::error_code ec;
    auto token = contract_handle.src_action_asset(ec);
    assert(!ec);

    EXPECT_EQ(token_amount, token.amount());
    EXPECT_EQ(token.symbol().to_string(), data::XPROPERTY_ASSET_TOP);
    token.clear();

    // second call will trigger assert
    ec.clear();
    // contract_handle.src_action_asset(ec);
    ASSERT_DEATH(contract_handle.src_action_asset(ec), "");
}

NS_END3