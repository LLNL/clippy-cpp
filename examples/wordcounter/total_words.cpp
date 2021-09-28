// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <metall/metall.hpp>
#include "count_table.hpp"

namespace mtl = metall;

int main(int argc, char **argv) {
  clippy::clippy clip("total_words", "Return the total number of words");
  clip.add_required<std::string>("path", "Data store path");

  clip.returns<int>("Total words");
  if (clip.parse(argc, argv)) { return 0; }

  auto path = clip.get<std::string>("path");

  // Cannot open the data store
  if (!mtl::manager::consistent(path.c_str())) {
    clip.to_return(0);
    return 0;
  }

  // Reattach the table using Metall
  mtl::manager manager(mtl::open_read_only, path.c_str());
  const auto[table, length] = manager.find<count_table_type>(mtl::unique_instance);

  if (length == 0) {
    // The table does not exist
    clip.to_return(0);
    return 0;
  }

  int num_total_words = 0;
  for (const auto &elem : *table) num_total_words += elem.second;
  clip.to_return(num_total_words);

  return 0;
}
