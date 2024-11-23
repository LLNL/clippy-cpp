add_test(TestGraph __init__)
add_test(TestGraph __str__)
add_test(TestGraph add_edge)
add_test(TestGraph add_node)
add_test(TestGraph nv)
add_test(TestGraph ne)
add_test(TestGraph degree)
add_test(TestGraph add_series)
add_test(TestGraph connected_components)
add_test(TestGraph drop_series)
add_custom_command(
        TARGET TestGraph_nv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/meta.json
                ${CMAKE_CURRENT_BINARY_DIR}/meta.json)
