# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

#
# This function adds a test.
#
function ( add_test class_name method_name )
  set(source "${method_name}.cpp")
  set(target "${class_name}_${method_name}")
  add_executable(${target} ${source})
  target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIRS})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  set_target_properties(${target} PROPERTIES OUTPUT_NAME "${method_name}" )
endfunction()



add_subdirectory(TestBag)
add_subdirectory(TestSet)
add_subdirectory(TestFunctions)
