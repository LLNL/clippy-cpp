// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <ygm/comm.hpp>

int main(int argc, char **argv) {
  ygm::comm world(&argc, &argv);
  {
    clippy::clippy clip("noop", "No operation");
    clip.returns<int>("#of ranks");
    if (clip.parse(argc, argv)) { return 0; }
    clip.to_return(world.size());
  }
  world.barrier();

  return 0;
}
