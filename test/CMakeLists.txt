# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

#
# This function adds a test.
#
function ( add_test test_name )
  set(example_source "${test_name}.cpp")
  set(example_exe "${test_name}")
  add_executable(${example_exe} ${example_source})
  target_include_directories(${example_exe} PRIVATE ${Boost_INCLUDE_DIRS})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

add_test(returns_string)
add_test(returns_int)
add_test(returns_bool)
add_test(returns_vec_int)
add_test(call_with_string)