// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <cstdlib>

int main(int argc, char **argv) {
  clippy::clippy clip("return_int",
                      "Always returns a (non-uniform) pseudo-random integer "
                      "between [0, max) (default 100)");

  clip.add_optional("max", "the maximum value of random number (right-open)",
                    100);
  clip.add_optional("fix42", "always return 42", false);

  if (clip.parse(argc, argv)) {
    return 0;
  }

  int v = (clip.get<bool>("fix42")) ? 42 : (std::rand() % clip.get<int>("max"));
  clip.to_return(v);

  return 0;
}
