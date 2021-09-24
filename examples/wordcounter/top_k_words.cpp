// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <metall/metall.hpp>
#include <queue>
#include <string>
#include <utility>
#include <algorithm>
#include "count_table.hpp"

namespace mtl = metall;

int main(int argc, char **argv) {
  clippy::clippy clip("top_k_words", "Return the top k words");
  clip.add_required<std::string>("path", "Data store path");
  clip.add_required<int>("k", "k");

  clip.returns<std::vector<std::string>>("Top k words");
  if (clip.parse(argc, argv)) { return 0; }

  auto path = clip.get<std::string>("path");
  auto k = clip.get<int>("k");

  // Cannot open the data store
  if (!mtl::manager::consistent(path.c_str())) {
    clip.to_return(std::vector<std::string>{});
    return 0;
  }

  // Reattach the table using Metall
  mtl::manager manager(mtl::open_read_only, path.c_str());
  const auto[table, length] = manager.find<count_table_type>(mtl::unique_instance);

  if (length == 0) {
    // The table does not exist
    clip.to_return(std::vector<std::string>{});
    return 0;
  }

  // Find the top k words
  std::priority_queue<std::pair<int, std::string>,
                      std::vector<std::pair<int, std::string>>,
                      std::greater<std::pair<int, std::string>>> top_k_queue;
  for (const auto &elem : *table) {
    top_k_queue.emplace(elem.second, elem.first.c_str());
    if (top_k_queue.size() > k) {
      top_k_queue.pop();
    }
  }

  // Show the top k world
  std::vector<std::string> top_k_words;
  while (!top_k_queue.empty()) {
    top_k_words.push_back(top_k_queue.top().second);
    top_k_queue.pop();
  }
  std::reverse(top_k_words.begin(), top_k_words.end());
  clip.to_return(top_k_words);

  return 0;
}
