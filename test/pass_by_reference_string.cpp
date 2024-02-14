// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("pass_by_reference_string", "Call with string");
  clip.add_required<std::string>("str", "Required String");
  
  if (clip.parse(argc, argv)) { return 0; }

    clip.overwrite_arg("str", std::string("Overwrote"));
  return 0;
}
