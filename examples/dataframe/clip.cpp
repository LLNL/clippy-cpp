
#include <iostream>
#include <fstream>
#include <sstream>

#include "clip.hpp"

#include <boost/json/src.hpp>

namespace boostjsn = boost::json;

void resultOK(std::ostream& os, const std::string& msg)
{
  os << "{\n  \"text\": \"OK\"";
  if (msg.size())
    os << ",\n  \"info\": \"" << msg << "\"";
  os << "\n}\n" << std::endl;
}

void errorJson(std::ostream& os, const std::string& msg)
{
  os << "{\n  \"text\": \"Err: " << msg << "!\"\n}\n" << std::endl;
}

namespace
{

/// from boost examples
void
pretty_print(std::ostream& os, boostjsn::value const& jv, std::string& indent)
{
    switch(jv.kind())
    {
      case boostjsn::kind::object:
      {
          os << "{\n";
          indent.append(4, ' ');
          auto const& obj = jv.get_object();
          if(! obj.empty())
          {
              auto it = obj.begin();
              for(;;)
              {
                  os << indent << boostjsn::serialize(it->key()) << " : ";
                  pretty_print(os, it->value(), indent);
                  if(++it == obj.end())
                      break;
                  os << ",\n";
              }
          }
          os << "\n";
          indent.resize(indent.size() - 4);
          os << indent << "}";
          break;
      }
  
      case boostjsn::kind::array:
      {
          os << "[\n";
          indent.append(4, ' ');
          auto const& arr = jv.get_array();
          if(! arr.empty())
          {
              auto it = arr.begin();
              for(;;)
              {
                  os << indent;
                  pretty_print( os, *it, indent);
                  if(++it == arr.end())
                      break;
                  os << ",\n";
              }
          }
          os << "\n";
          indent.resize(indent.size() - 4);
          os << indent << "]";
          break;
      }
  
      case boostjsn::kind::string:
      {
          os << boostjsn::serialize(jv.get_string());
          break;
      }
  
      case boostjsn::kind::uint64:
          os << jv.get_uint64();
          break;
  
      case boostjsn::kind::int64:
          os << jv.get_int64();
          break;
  
      case boostjsn::kind::double_:
          os << jv.get_double();
          break;
  
      case boostjsn::kind::bool_:
          if(jv.get_bool())
              os << "true";
          else
              os << "false";
          break;
  
      case boostjsn::kind::null:
          os << "null";
          break;
      }
  
      if(indent.empty())
          os << "\n";
}
}


boostjsn::value
parseFile(std::istream& inps)
{
  boostjsn::stream_parser p;
  std::string             line;
  
  while (inps >> line)
  {
    boostjsn::error_code ec;
    
    p.write(line.c_str(), line.size(), ec);
    
    if (ec) return nullptr;
  } 
  
  boostjsn::error_code ec;
  p.finish(ec);
  if (ec) return nullptr;
  
  return p.release();
}

boostjsn::value
parseFile(const std::string& filename)
{
  std::ifstream is{filename};
  
  return parseFile(is);
}

void
prettyPrint(std::ostream& os, const boostjsn::value& jv)
{
  std::string   indent;
  
  pretty_print(os, jv, indent);
}

void
prettyPrint(const boostjsn::value& jv)
{
  std::ofstream os{"jsonin.log"};
  
  prettyPrint(os, jv);
}

void setValueIfAvail(const boostjsn::value& args, const char* key, std::string& name)
try
{
  const boostjsn::object&          argsobj = args.as_object();
  boostjsn::object::const_iterator pos     = argsobj.find(key);
  if (pos == argsobj.end()) return;
  
  name = pos->value().as_string().c_str();
}
catch (const std::invalid_argument&) {}

void setValueIfAvail(const boostjsn::value& args, const char* key, int& val)
try
{
  const boostjsn::object&          argsobj = args.as_object();
  boostjsn::object::const_iterator pos     = argsobj.find(key);
  if (pos == argsobj.end()) return;
  
  val = pos->value().as_int64();
}
catch (const std::invalid_argument&) {}


int columnIndex(const std::vector<std::string>& all, const std::string& colname)
{
  std::vector<std::string>::const_iterator aa  = all.begin();
  std::vector<std::string>::const_iterator zz  = all.end();
  std::vector<std::string>::const_iterator pos = std::find(aa, all.end(), colname);
  
  clippy_assert(pos != zz, "Column name not found: " + colname);
  return std::distance(aa, pos);
}

bool fail( const std::string& msg
#if __cpp_lib_source_location
         , std::source_location pos = std::source_location::current()
#endif /* __cpp_lib_source_location */
         )
{
  std::stringstream errmsg;

  errmsg << msg
#if __cpp_lib_source_location
         << " @ " << pos.file_name
         << " : " << pos.line
#endif /* __cpp_lib_source_location */
         ;

  throw std::logic_error{errmsg.str()};
}
