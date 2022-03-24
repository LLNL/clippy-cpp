# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall -Wextra -pedantic)

include_directories(${Boost_INCLUDE_DIRS})

add_example(vector_query)
