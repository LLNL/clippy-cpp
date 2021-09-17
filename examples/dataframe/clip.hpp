
#pragma once

#include <string>
#include <iosfwd>
#include <memory>
#include <sstream>
//~ #include <source_location>

#include <boost/json.hpp>

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

bool fail(const std::string& msg);

namespace
{
  template <class EX = std::logic_error>
  inline
  void clippy_assert(bool cond, const std::string& msg) // , std::source_location pos = std::source_location::current())
  {
    if (cond) return;
    
    std::stringstream errmsg;
    
    errmsg << msg 
           //~ << " @ " << pos.file_name 
           //~ << ":" << pos.line
           ;
           
    throw EX{errmsg.str()};
  }
}
