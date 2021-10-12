// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbasic/xerror/xerror.h"

#include <string>
#include <system_error>

NS_BEG1(top)

template <typename T>
std::string to_string(T const & input) {
    return input.to_string();
}

template <typename T>
T from_string(std::string const & input, std::error_code & ec) {
    T ret;
    ret.from_string(input, ec);
    return ret;
}

template <typename T>
T from_string(std::string const & input) {
    std::error_code ec;
    auto ret = top::from_string<T>(input, ec);
    top::error::throw_error(ec);
    return ret;
}

template <>
std::string to_string<int>(int const & input);

template <>
std::string to_string<long>(long const & input);

template <>
std::string to_string<long long>(long long const & input);

template <>
std::string to_string<unsigned long>(unsigned long const & input);

template <>
std::string to_string<unsigned long long>(unsigned long long const & input);

template <>
std::string to_string<std::string>(std::string const & input);

template <>
int from_string<int>(std::string const & input);

template <>
long from_string<long>(std::string const & input);

template <>
long long from_string<long long>(std::string const & input);

template <>
unsigned long from_string<unsigned long>(std::string const & input);

template <>
unsigned long long from_string<unsigned long long>(std::string const & input);

template <>
float from_string<float>(std::string const & input);

template <>
double from_string<double>(std::string const & input);

template <>
long double from_string<long double>(std::string const & input);

NS_END1
