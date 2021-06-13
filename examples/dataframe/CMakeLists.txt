# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

function ( add_df_example example_name )
  add_example(${example_name})
  target_link_libraries(${example_name} libcliprt)
  endfunction()

include_directories(${Boost_INCLUDE_DIRS}) 

add_library(libcliprt STATIC clip)
target_link_libraries(libcliprt ${Boost_LIBRARIES})

add_df_example(columninfo)
add_df_example(columnquery)
add_df_example(create)
add_df_example(datafind)
add_df_example(importCSV)
add_df_example(rowquery)

