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

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);
  {
    {
      clippy::clippy clip("noop", "");
      clip.returns<std::string>("Size");
      if (clip.parse(argc, argv)) { return 0; }

      int mpi_size;
      MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

      clip.to_return(mpi_size);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  MPI_Finalize();

  return 0;
}
