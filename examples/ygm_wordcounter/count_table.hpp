// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#ifndef CLIPPY_EXAMPLES_YGM_YGM_COUNT_TABLE_HPP_
#define CLIPPY_EXAMPLES_YGM_YGM_COUNT_TABLE_HPP_

#include <functional>
#include <metall/container/string.hpp>
#include <metall/container/unordered_map.hpp>
#include <metall/container/scoped_allocator.hpp>
#include <metall/utility/hash.hpp>
#include <metall/utility/fallback_allocator_adaptor.hpp>

namespace ygm_wordcount {
namespace {
namespace mc = metall::container;
}

template <typename T>
using alloc_t = metall::manager::allocator_type<T>;

template <typename T>
using sc_alloc_t = mc::scoped_allocator_adaptor<alloc_t<T>>;

using pmem_str_t = mc::basic_string<char, std::char_traits<char>, sc_alloc_t<char>>;
using pmem_count_table = mc::unordered_map<pmem_str_t,
                                           std::size_t,
                                           metall::utility::string_hash<pmem_str_t, 12345>,
                                           std::equal_to<>,
                                           sc_alloc_t<std::pair<pmem_str_t, std::size_t>>>;
}

#endif //CLIPPY_EXAMPLES_YGM_YGM_COUNT_TABLE_HPP_
