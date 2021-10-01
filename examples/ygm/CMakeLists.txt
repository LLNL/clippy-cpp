if (MPI_CXX_FOUND)
    add_example(ygm_wordcount)

    target_include_directories(ygm_wordcount PUBLIC ${cereal_INCLUDE_DIR})
    target_include_directories(ygm_wordcount PUBLIC ${YGM_INCLUDE_DIR})
    target_include_directories(ygm_wordcount PUBLIC ${MPI_INCLUDE_PATH})

    target_link_libraries(ygm_wordcount PRIVATE MPI::MPI_CXX)
else()
    message(STATUS "Will skip building the YGM examples")
endif()