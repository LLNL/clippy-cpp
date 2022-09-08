# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

add_compile_options(-Wall -Wextra -pedantic -ggdb -O0)

include_directories(${Boost_INCLUDE_DIRS})

add_example(testeval)

