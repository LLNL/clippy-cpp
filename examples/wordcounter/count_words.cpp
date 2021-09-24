// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <metall/metall.hpp>
#include "count_table.hpp"

namespace mtl = metall;

int main(int argc, char **argv) {
  clippy::clippy clip("count_words", "Count words");
  clip.add_required<std::string>("path","Data store path");
  clip.add_required<std::vector<std::string>>("words","Unordered array of words");

  clip.returns<int>("Total words");
  if (clip.parse(argc, argv)) { return 0; }

  auto words = clip.get<std::vector<std::string>>("words");
  auto path = clip.get<std::string>("path");

  std::unique_ptr<mtl::manager> manager;
  if (mtl::manager::consistent(path.c_str())) { // Does the data store already exist?
    // Open an existing data store
    manager = std::make_unique<mtl::manager>(mtl::open_only, path.c_str());
  } else {
    // Create a new data store
    manager = std::make_unique<mtl::manager>(mtl::create_only, path.c_str());
  }
  // Find the existing table or construct a new one
  auto table = manager->find_or_construct<count_table_type>(mtl::unique_instance)(manager->get_allocator());

  // Count words
  for (const auto &word : words) {
    auto pos = table->try_emplace(mtl::container::string(word.c_str(), table->get_allocator()), 0);
    ++(pos.first->second);
  }

  int num_total_words = 0;
  for (const auto& elem : *table) num_total_words += elem.second;
  clip.to_return(num_total_words);

  return 0;
}
