#include "xdata/xfulltableblock_account_data.h"

#include "xvledger/xvblock.h"
#include "xbase/xobject_ptr.h"

NS_BEG2(top, data)

int32_t xtop_fulltableblock_account_data::do_read(base::xstream_t & stream) {
    auto const size = stream.size();
    stream >> account_detail;
    return size - stream.size();
}

int32_t xtop_fulltableblock_account_data::do_write(base::xstream_t & stream) const {
    auto const size = stream.size();
    stream << account_detail;
    return stream.size() - size;
}

int32_t operator>>(base::xstream_t& stream, xfulltableblock_account_data_t& data_object) {
    return data_object.serialize_from(stream);
}
int32_t operator<<(base::xstream_t& stream, xfulltableblock_account_data_t const& data_object) {
    return data_object.serialize_to(stream);
}

int32_t operator>>(base::xbuffer_t & buffer, xfulltableblock_account_data_t& data_object) {
    return data_object.serialize_from(buffer);
}
int32_t operator<<(base::xbuffer_t & buffer, xfulltableblock_account_data_t const& data_object) {
    return data_object.serialize_to(buffer);
}

int32_t xtop_fulltableblock_group_data::do_read(base::xstream_t & stream) {
    auto const size = stream.size();
    stream >> account_data;
    return size - stream.size();
}

int32_t xtop_fulltableblock_group_data::do_write(base::xstream_t & stream) const {
    auto const size = stream.size();
    stream << account_data;
    return stream.size() - size;
}

int32_t operator>>(base::xstream_t& stream, xfulltableblock_group_data_t& data_object) {
    return data_object.serialize_from(stream);
}
int32_t operator<<(base::xstream_t& stream, xfulltableblock_group_data_t const& data_object) {
    return data_object.serialize_to(stream);
}

int32_t operator>>(base::xbuffer_t & buffer, xfulltableblock_group_data_t& data_object) {
    return data_object.serialize_from(buffer);
}
int32_t operator<<(base::xbuffer_t & buffer, xfulltableblock_group_data_t const& data_object) {
    return data_object.serialize_to(buffer);
}





NS_END2