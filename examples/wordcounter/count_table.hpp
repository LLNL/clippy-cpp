// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#ifndef CLIPPY_EXAMPLES_WORDCOUNTER_COUNT_TABLE_HPP
#define CLIPPY_EXAMPLES_WORDCOUNTER_COUNT_TABLE_HPP

#include <functional>
#include <metall/container/string.hpp>
#include <metall/container/unordered_map.hpp>
#include <metall/utility/hash.hpp>

using table_value_type = std::pair<metall::container::string, int>;
using count_table_type = metall::container::unordered_map<metall::container::string, int,
                                                          metall::utility::string_hash<metall::container::string>,
                                                          std::equal_to<metall::container::string>,
                                                          metall::manager::scoped_allocator_type<table_value_type>>;

#endif //CLIPPY_EXAMPLES_WORDCOUNTER_COUNT_TABLE_HPP
