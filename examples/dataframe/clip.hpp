
#pragma once

#include <string>
#include <iosfwd>
#include <memory>
#include <sstream>

#if defined __has_include
  #if __has_include (<source_location>)
  #include <source_location>
  #endif
#endif /* defined __has_include */

#include <boost/json.hpp>

#include "experimental/cxx-compat.hpp"

constexpr const bool firstMatch = false;

void resultOK(std::ostream& os, const std::string& msg = {});
void errorJson(std::ostream& os, const std::string& msg);

void prettyPrint(std::ostream& os, boost::json::value const& jv);
void prettyPrint(boost::json::value const& jv);

boost::json::value parseFile(std::istream& is);
boost::json::value parseFile(const std::string& filename);

void setValueIfAvail(const boost::json::value& args, const char* key, std::string& name);
void setValueIfAvail(const boost::json::value& args, const char* key, int& val);

int columnIndex(const std::vector<std::string>& all, const std::string& colname);

CXX_NORETURN
bool fail( const std::string& msg
#if __cpp_lib_source_location
         , std::source_location pos = std::source_location::current()
#endif /* __cpp_lib_source_location */
         );

namespace
{
  inline
  void clippy_assert( bool cond
                    , const std::string& msg
#if __cpp_lib_source_location
                    , std::source_location pos = std::source_location::current()
#endif /* __cpp_lib_source_location */
                    )
  {
    if (cond) { CXX_LIKELY; return; }
    
    fail( msg
#if __cpp_lib_source_location
        , pos
#endif /* __cpp_lib_source_location */
        );
  }
}
