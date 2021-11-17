// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include "xbase/xutl.h"
#include "xdata/xlightunit_info.h"
#include "xdata/xdata_common.h"

namespace top { namespace data {

xtransaction_exec_state_t::xtransaction_exec_state_t()
: xblockpara_base_t() {

}

xtransaction_exec_state_t::xtransaction_exec_state_t(const std::map<std::string, std::string> & values)
: xblockpara_base_t(values) {

}

void xtransaction_exec_state_t::set_tx_exec_status(enum_xunit_tx_exec_status value) {
    set_value(XTX_STATE_TX_EXEC_STATUS, (uint32_t)value);
}

void xtransaction_exec_state_t::set_receipt_id(base::xtable_shortid_t self_tableid, base::xtable_shortid_t peer_tableid, uint64_t receiptid) {
    set_value(XTX_RECEIPT_ID, receiptid);
    set_value(XTX_RECEIPT_ID_SELF_TABLE_ID, self_tableid);
    set_value(XTX_RECEIPT_ID_PEER_TABLE_ID, peer_tableid);
}

void xtransaction_exec_state_t::set_sender_confirmed_receipt_id(uint64_t receiptid) {
    set_value(XTX_SENDER_CONFRIMED_RECEIPT_ID, receiptid);
}

void xtransaction_exec_state_t::set_receipt_data(xreceipt_data_t data) {
    if (!data.empty()) {
        base::xstream_t stream(base::xcontext_t::instance());
        MAP_SERIALIZE_SIMPLE(stream, data);
        set_value(XTX_RECEIPT_DATA, std::string{(char*)stream.data(), (uint32_t)stream.size()});
    }
}

xreceipt_data_t  xtransaction_exec_state_t::get_receipt_data() const {
    auto const& res = get_value(XTX_RECEIPT_DATA);
    if (res.empty()) return xreceipt_data_t{};

    base::xstream_t stream(base::xcontext_t::instance(), (uint8_t*)res.data(), (int32_t)res.size());
    xreceipt_data_t data;
    MAP_DESERIALIZE_SIMPLE(stream, data);
    return data;
}

enum_xunit_tx_exec_status xtransaction_exec_state_t::get_tx_exec_status() const {
    enum_xunit_tx_exec_status status = static_cast<enum_xunit_tx_exec_status>(get_value_uint32(XTX_STATE_TX_EXEC_STATUS));
    return status;
}

uint64_t xlightunit_tx_info_t::get_last_trans_nonce() const {
    if (is_self_tx() || is_send_tx()) {
        if (m_raw_tx != nullptr) {
            return m_raw_tx->get_last_nonce();
        }
    }
    return 0;
}



}  // namespace data
}  // namespace top
