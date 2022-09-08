# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

add_compile_options(-Wall -Wextra -pedantic)

include_directories(${Boost_INCLUDE_DIRS})

add_example(df-init)
add_example(df-importFile)
add_example(df-metadata)
add_example(df-extreme)
add_example(df-rowquery)
add_example(df-identity)

