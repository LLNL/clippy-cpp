// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <string>
#include <queue>
#include <vector>
#include <utility>
#include <clippy/clippy.hpp>
#include <ygm/comm.hpp>
#include <metall/utility/metall_mpi_adaptor.hpp>

#include "count_table.hpp"

using namespace ygm_wordcount;

// The top one has the lowest count
using que_type = std::priority_queue<std::pair<std::size_t, std::string>,
                                     std::vector<std::pair<std::size_t, std::string>>,
                                     std::greater<std::pair<std::size_t, std::string>>>;

static int k = 0;

int main(int argc, char **argv) {
  ygm::comm world(&argc, &argv);

  {
    clippy::clippy clip("top_k_words", "Return the top k words");
    clip.add_required<std::string>("path", "Data store path");
    clip.add_required<int>("k", "k");
    clip.returns<std::vector<std::string>>("Top k words");
    if (clip.parse(argc, argv)) { return 0; }

    const auto path = clip.get<std::string>("path") + "-" + std::to_string(world.rank());
    k = clip.get<int>("k");

    std::unique_ptr<metall::manager> manager;
    manager = std::make_unique<metall::manager>(metall::open_read_only, path.c_str());
    static const pmem_count_table *table = manager->find<pmem_count_table>(metall::unique_instance).first;

    // Find the local top k words
    que_type local_top_k_queue;
    for (const auto &elem: *table) {
      if (local_top_k_queue.size() < k || local_top_k_queue.top().first < elem.second) {
        local_top_k_queue.emplace(elem.second, elem.first.c_str());
      }
      if (local_top_k_queue.size() > k) {
        local_top_k_queue.pop();
      }
    }

    // Find the global top k words
    static que_type global_top_k_queue;
    while (!local_top_k_queue.empty()) {
      auto gather = [](auto pcomm, int from, const std::size_t count, const std::string &word) {
        if (global_top_k_queue.size() < k || global_top_k_queue.top().first < count) {
          global_top_k_queue.emplace(count, word);
        }
        if (global_top_k_queue.size() > k) {
          global_top_k_queue.pop();
        }
      };
      const auto &top = local_top_k_queue.top();
      world.async(0, gather, top.first, top.second);
      local_top_k_queue.pop();
    }

    world.barrier();

    std::vector<std::string> top_k_words;
    while (!global_top_k_queue.empty()) {
      top_k_words.emplace_back(global_top_k_queue.top().second);
      global_top_k_queue.pop();
    }
    std::reverse(top_k_words.begin(), top_k_words.end());
    clip.to_return(top_k_words);
    table = nullptr;
  }
  world.barrier();

  return 0;
}
