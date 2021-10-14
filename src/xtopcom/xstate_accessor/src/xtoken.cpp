// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "xstate_accessor/xtoken.h"

#include "xbasic/xerror/xerror.h"
#include "xcontract_common/xerror/xerror.h"
#include "xutility/xhash.h"

#include <cassert>
#include <memory>

NS_BEG2(top, state_accessor)

xtop_token::xtop_token(xtop_token && other) noexcept : value_{ other.value_ }, symbol_{ std::move(other.symbol_) } {
    other.value_ = 0;
    other.symbol_.clear();
}

xtop_token::xtop_token(common::xsymbol_t symbol) : symbol_{ std::move(symbol) } {
}

xtop_token::xtop_token(std::uint64_t const amount, common::xsymbol_t symbol) : value_{ amount }, symbol_ { std::move(symbol) } {
}

xtop_token::xtop_token(std::uint64_t const amount) : value_{amount} {
}

xtop_token::~xtop_token() noexcept {
    assert(value_ == 0);
    assert(symbol_.empty());
}

bool xtop_token::operator==(xtop_token const & other) const noexcept {
    if (symbol() != other.symbol()) {
        return false;
    }

    if (invalid()) {
        assert(other.invalid());
        return true;
    }

    return value_ == other.value_;
}

bool xtop_token::operator!=(xtop_token const & other) const noexcept {
    return !(*this == other);
}

bool xtop_token::operator<(xtop_token const & other) const {
    if (other.invalid()) {
        return false;
    }

    if (invalid()) {
        return true;
    }

    if (symbol() != other.symbol()) {
        top::error::throw_error(error::xerrc_t::token_symbol_not_matched, "xtoken_t::operator<=>(xtoken_t const & other)");
    }

    return value_ < other.value_;
}

bool xtop_token::operator>(xtop_token const & other) const {
    return other < *this;
}

bool xtop_token::operator<=(xtop_token const & other) const {
    return !(*this > other);
}

bool xtop_token::operator>=(xtop_token const & other) const {
    return !(*this < other);
}

xtop_token & xtop_token::operator+=(xtop_token & other) noexcept {
    if (std::addressof(other) == std::addressof(*this)) {
        return *this;
    }

    if (symbol() != other.symbol()) {
        return *this;
    }

    if (invalid()) {
        assert(other.invalid());
        return *this;
    }

    value_ += other.value_;
    other.value_ = 0;

    return *this;
}

bool xtop_token::invalid() const noexcept {
    return symbol_.empty();
}

uint64_t xtop_token::amount() const noexcept {
    return value_;
}

common::xsymbol_t const & xtop_token::symbol() const noexcept {
    return symbol_;
}

void xtop_token::clear() noexcept {
    value_ = 0;
    symbol_.clear();
}

NS_END2

NS_BEG1(std)

size_t hash<top::state_accessor::xtoken_t>::operator()(top::state_accessor::xtoken_t const & amount) const noexcept {
    top::utl::xxh64_t xxhash;
    auto const value = amount.amount();

    xxhash.update(std::addressof(value), sizeof(value));
    xxhash.update(amount.symbol().to_string());

    return static_cast<size_t>(xxhash.get_hash());
}

NS_END1
