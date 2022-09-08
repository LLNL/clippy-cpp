# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall -Wextra -pedantic)

include_directories(${Boost_INCLUDE_DIRS})

add_example(howdy-init)
add_example(howdy-greet)
add_example(howdy-setGreeting)
add_example(howdy-setGreeted)
add_example(howdy-getitem)
add_example(howdy-eval)

