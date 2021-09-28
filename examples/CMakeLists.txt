# Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
# Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: MIT

#
# This function adds an mpi example.
#

# PP: added because I got an error "Could NOT find Threads (missing: Threads_FOUND)"
#     solution according to: https://github.com/alicevision/geogram/issues/2
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

function ( add_example example_name )
  set(example_source "${example_name}.cpp")
  set(example_exe "${example_name}")
  add_executable(${example_exe} ${example_source})
  target_include_directories(${example_exe} PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(${example_exe} PRIVATE Metall)
  target_link_libraries(${example_exe} PRIVATE stdc++fs Threads::Threads)
  if (UNIX AND NOT APPLE)
    target_link_libraries(${example_exe} PRIVATE rt)
  endif ()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

add_example(grumpy)
add_example(sort_edges)
add_example(sort_strings)
add_example(howdy)
add_example(sum)
add_example(return_tuple)
add_example(sort_string_edges)

add_subdirectory(wordcounter)
add_subdirectory(dataframe)
add_subdirectory(mpi)
add_subdirectory(ygm_wordcounter)
