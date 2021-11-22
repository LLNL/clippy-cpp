#pragma once

#if __cplusplus >= 202002L

#if __has_cpp_attribute(carries_dependency)
#define CXX_CARRIES_DEPENDENCY [[carries_dependency]]
#endif

#if __has_cpp_attribute(deprecated)
#define CXX_DEPRECATED [[deprecated]]
#endif

#if __has_cpp_attribute(fallthrough)
#define CXX_FALLTHROUGH [[fallthrough]]
#endif

#if __has_cpp_attribute(likely)
#define CXX_LIKELY [[likely]]
#endif

#if __has_cpp_attribute(maybe_unused)
#define CXX_MAYBE_UNUSED [[maybe_unused]]
#endif

#if __has_cpp_attribute(no_unique_address)
#define CXX_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if __has_cpp_attribute(nodiscard)
#define CXX_NODISCARD [[nodiscard]]
#endif

#if __has_cpp_attribute(noreturn)
#define CXX_NORETURN [[noreturn]]
#endif

#if __has_cpp_attribute(unlikely)
#define CXX_UNLIKELY [[unlikely]]
#endif

#endif /* C++20 */


#ifndef CXX_CARRIES_DEPENDENCY
#define CXX_CARRIES_DEPENDENCY
#endif

#ifndef CXX_DEPRECATED
#define CXX_DEPRECATED
#endif

#ifndef CXX_FALLTHROUGH
#define CXX_FALLTHROUGH
#endif

#ifndef CXX_LIKELY
#define CXX_LIKELY
#endif

#ifndef CXX_MAYBE_UNUSED
#define CXX_MAYBE_UNUSED
#endif

#ifndef CXX_NO_UNIQUE_ADDRESS
#define CXX_NO_UNIQUE_ADDRESS
#endif

#ifndef CXX_NODISCARD
#define CXX_NODISCARD
#endif

#ifndef CXX_NORETURN
#define CXX_NORETURN
#endif

#ifndef CXX_UNLIKELY
#define CXX_UNLIKELY
#endif





