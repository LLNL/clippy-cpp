function(prepare_fetchcontent_boost)
    set(BOOST_INCLUDE_LIBRARIES json PARENT_SCOPE)
    set(BOOST_ENABLE_CMAKE ON PARENT_SCOPE)

    set(BOOST_REQD_SUBMODULES
            "tools/cmake;"
            "libs/assert;"
            "libs/exception;"
            "libs/throw_exception;"
            "libs/static_assert;"
            "libs/config;"
            "libs/container;"
            "libs/container_hash;"
            "libs/utility;"
            "libs/type_traits;"
            "libs/move;"
            "libs/tuple;"
            "libs/variant2;"
            "libs/detail;"
            "libs/smart_ptr;"
            "libs/integer;"
            "libs/intrusive;"
            "libs/io;"
            # "libs/iostreams;"
            "libs/describe;"
            "libs/core;"
            "libs/align;"
            "libs/predef;"
            "libs/preprocessor;"
            "libs/system;"
            "libs/winapi;"
            "libs/mp11;"
            "libs/json;"
            # "libs/property_tree;"
            # "libs/interprocess;"
            # "libs/optional;"
            # "libs/any;"
            # "libs/type_index;"
            # "libs/mpl;"
            # "libs/multi_index;"
            # "libs/serialization;"
            # "libs/bind;"
            # "libs/foreach;"
            # "libs/iterator;"
            # "libs/headers;"
            # "libs/format;"
            # "libs/range;"
            # "libs/concept_check;"
            # "libs/uuid;"
            # "libs/random;"
            # "libs/tti;"
            # "libs/function_types;"
            PARENT_SCOPE)
endfunction()

function(get_boost_include_dirs)
    list(APPEND BOOST_INCLUDE_DIRS
            # ${Boost_SOURCE_DIR}/libs/interprocess/include
            # ${Boost_SOURCE_DIR}/libs/property_tree/include
            # ${Boost_SOURCE_DIR}/libs/optional/include
            # ${Boost_SOURCE_DIR}/libs/any/include
            # ${Boost_SOURCE_DIR}/libs/type_index/include
            # ${Boost_SOURCE_DIR}/libs/mpl/include
            # ${Boost_SOURCE_DIR}/libs/bind/include
            # ${Boost_SOURCE_DIR}/libs/multi_index/include
            # ${Boost_SOURCE_DIR}/libs/serialization/include
            # ${Boost_SOURCE_DIR}/libs/foreach/include
            # ${Boost_SOURCE_DIR}/libs/iterator/include
            ${Boost_SOURCE_DIR}/libs/container/include
            # ${Boost_SOURCE_DIR}/libs/unordered/include
            # ${Boost_SOURCE_DIR}/libs/iostreams/include
            ${Boost_SOURCE_DIR}/libs/system/include
            ${Boost_SOURCE_DIR}/libs/describe/include
            # ${Boost_SOURCE_DIR}/libs/format/include
            # ${Boost_SOURCE_DIR}/libs/range/include
            # ${Boost_SOURCE_DIR}/libs/concept_check/include
            # ${Boost_SOURCE_DIR}/libs/uuid/include
            # ${Boost_SOURCE_DIR}/libs/random/include
            # ${Boost_SOURCE_DIR}/libs/tti/include
            # ${Boost_SOURCE_DIR}/libs/function_types/include
            )
    set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS} PARENT_SCOPE)
endfunction()
