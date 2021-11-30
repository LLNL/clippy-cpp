// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <string>
#include <functional>
#include <fstream>

#include <clippy/clippy.hpp>
#include <ygm/comm.hpp>
#include <metall/metall.hpp>

#include "count_table.hpp"

using namespace ygm_wordcount;

int main(int argc, char **argv) {
  ygm::comm world(&argc, &argv, 8 * 1024);

  {
    clippy::clippy clip("wordcount", "Distributed word count example");
    clip.add_required<std::string>("path", "Data store path");
    clip.add_required<std::vector<std::string>>("files", "Input word files");
    clip.returns<std::size_t>("Total unique worlds");
    if (clip.parse(argc, argv)) { return 0; }

    const auto path = clip.get<std::string>("path") + "-" + std::to_string(world.rank());

    std::unique_ptr<metall::manager> manager;
    if (metall::manager::consistent(path.c_str())) {
      manager = std::make_unique<metall::manager>(metall::open_only, path.c_str());
    } else {
      manager = std::make_unique<metall::manager>(metall::create_only, path.c_str());
    }
    static pmem_count_table *table = manager->find_or_construct<pmem_count_table>(metall::unique_instance)(manager->get_allocator());

    const auto word_files = clip.get<std::vector<std::string>>("files");
    for (int i = 0; i < word_files.size(); ++i) {
      if (i % world.size() != world.rank()) continue;

      std::ifstream ifs(word_files[i]);
      std::string word;
      while (ifs >> word) {
        auto counter = [](auto pcomm, int from, const std::string &w) {
          pmem_str_t tmp(w.c_str(), table->get_allocator());
          if (table->count(tmp) == 0) {
            table->emplace(std::move(tmp), 1);
          } else {
            table->at(tmp)++;
          }
        };
        const int dest = metall::utility::string_hash<std::string, 321>{}(word) % world.size();
        world.async(dest, counter, word);
      }
    }
    world.barrier();

    const auto total_words = world.all_reduce_sum(table->size());
    clip.to_return(total_words);
    table = nullptr;
  }
  world.barrier();

  return 0;
}
