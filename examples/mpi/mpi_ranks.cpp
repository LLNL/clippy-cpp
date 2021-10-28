// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <string>
#include <mpi.h>

#ifndef MPI_VERSION
#error "MPI_VERSION is not defined."
#endif

static constexpr int k_local_buf_size = 1024;

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);
  {
    clippy::clippy clip("ranks", "Lists MPI ranks");
    clip.returns<std::string>("Ranks");
    if (clip.parse(argc, argv)) { return 0; }

    int mpi_size;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    std::vector<int> ranks(mpi_size);
    MPI_Gather(&mpi_rank, 1, MPI_INT, ranks.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    clip.to_return(ranks);
  }
  MPI_Finalize();

  return 0;
}
