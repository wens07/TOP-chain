// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push, 0)
#endif

#include "xbase/xmem.h"

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

#include "xbasic/xerror/xthrow_error.h"
#include "xbasic/xutility.h"
#include "xcontract_runtime/xerror/xerror.h"

#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

NS_BEG3(top, contract_runtime, system)

template <typename StreamT>
struct Functor {
    Functor(StreamT & ds) : m_ds(ds) {
    }

    template <typename T>
    void operator()(T & t) const {
        m_ds >> t;
    }

private:
    StreamT & m_ds;
};

template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp)>::type for_each(std::tuple<Tp...> &, FuncT) {
}

template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<(I < sizeof...(Tp))>::type for_each(std::tuple<Tp...> & t, FuncT f) {
    f(std::get<I>(t));
    for_each<I + 1, FuncT, Tp...>(t, f);
}

template <typename... Args>
base::xstream_t & operator>>(base::xstream_t & ds, std::tuple<Args...> & t) {
    for_each(t, Functor<base::xstream_t>(ds));
    return ds;
}

template <typename Callable, typename Obj, typename Tuple, std::size_t ... I>
auto do_call(Callable && func, Obj && obj, Tuple && tuple, top::index_sequence<I...>)
    -> decltype(func(std::forward<Obj>(obj), std::get<I>(std::forward<Tuple>(tuple))...)) {
    return func(std::forward<Obj>(obj), std::get<I>(std::forward<Tuple>(tuple))...);
}

template <typename Callable, typename T, typename Tuple>
auto call(Callable && func, T && obj, Tuple && tuple) -> decltype(do_call(std::forward<Callable>(func),
                                                                          std::forward<T>(obj),
                                                                          std::forward<Tuple>(tuple),
                                                                          top::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{})) {
    return do_call(std::forward<Callable>(func), std::forward<T>(obj), std::forward<Tuple>(tuple), top::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{});
}

template <typename T>
T unpack(base::xstream_t & stream) {
    T result;
    stream >> result;
    return result;
}

template <typename ContractT, typename Callable, typename... Args>
void call_contract_api(ContractT * obj, top::base::xstream_t & stream, Callable && func, void (ContractT::*)(Args...)) {
    call(std::forward<Callable>(func), obj, unpack<std::tuple<typename std::decay<Args>::type...>>(stream));
}

/// @brief Call contract API marco
///        action_name is from 'BEGIN_CONTRACT_API'
#define CALL_CONTRACT_API(CONTRACT_API)                                                                                                                                            \
    do {                                                                                                                                                                           \
        std::string const api_string{#CONTRACT_API};                                                                                                                               \
        auto const pos = api_string.find("::");                                                                                                                                    \
        if (pos == std::string::npos) {                                                                                                                                            \
            break;                                                                                                                                                                 \
        }                                                                                                                                                                          \
        auto const api_name = api_string.substr(pos + 2);                                                                                                                          \
        if (action_name != api_name) {                                                                                                                                             \
            break;                                                                                                                                                                 \
        }                                                                                                                                                                          \
        top::contract_common::xcontract_execution_result_t result;                                                                                                                 \
        try {                                                                                                                                                                      \
            top::contract_runtime::system::call_contract_api(this, stream, std::mem_fn(&CONTRACT_API), &CONTRACT_API);                                                             \
        } catch (top::error::xtop_error_t const & eh) {                                                                                                                            \
            result.status.ec = eh.code();                                                                                                                                          \
            result.status.extra_msg = eh.what();                                                                                                                                   \
        } catch (std::exception const & eh) {                                                                                                                                      \
            result.status.ec = top::contract_runtime::error::xerrc_t::unknown_error;                                                                                               \
            result.status.extra_msg = eh.what();                                                                                                                                   \
        } catch (...) {                                                                                                                                                            \
            result.status.ec = top::contract_runtime::error::xerrc_t::unknown_error;                                                                                               \
        }                                                                                                                                                                          \
        return result;                                                                                                                                                             \
    } while (false)

/// @brief Macro for declaring (calling) contract API 'func'
///        'CONTRACT' is from 'BEGIN_CONTRACT_APIs'
#define DECLARE_API(func) CALL_CONTRACT_API(func)

/// @brief  contract begin & end macro
#define BEGIN_CONTRACT_API()                                                                                                                                                       \
    top::contract_common::xcontract_execution_result_t execute(observer_ptr<top::contract_common::xcontract_execution_context_t> exe_ctx) override {                               \
        this->reset_execution_context(exe_ctx);                                                                                                                                    \
        auto const & action_name = exe_ctx->action_name();                                                                                                                         \
        auto const & action_data = exe_ctx->action_data();                                                                                                                         \
        base::xstream_t stream(base::xcontext_t::instance(), (uint8_t *)action_data.data(), action_data.size());

#define END_CONTRACT_API                                                                                                                                                           \
        top::error::throw_error(top::contract_runtime::error::xerrc_t::contract_api_not_found);                                                                                    \
        return {};                                                                                                                                                                 \
    }

#define XCONTRACT_ENSURE(condition, msg)                                                                                                                                           \
    do {                                                                                                                                                                           \
        if (!(condition)) {                                                                                                                                                        \
            std::error_code ec{contract_runtime::error::xenum_errc::enum_vm_exception};                                                                                            \
            top::error::throw_error(ec, msg);                                                                                                                                      \
        }                                                                                                                                                                          \
    } while (false)

NS_END3