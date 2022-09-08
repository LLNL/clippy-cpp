# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

function ( add_df_example example_name )
  add_example(${example_name})
  target_link_libraries(${example_name} PRIVATE libcliprt)
  endfunction()

include_directories(${Boost_INCLUDE_DIRS}) 

add_library(libcliprt STATIC clip.cpp)
target_link_libraries(libcliprt PUBLIC ${Boost_LIBRARIES})

add_df_example(metadataquery)
add_df_example(columnquery)
add_df_example(create)
add_df_example(extreme)
add_df_example(importCSV)
add_df_example(rowquery)

