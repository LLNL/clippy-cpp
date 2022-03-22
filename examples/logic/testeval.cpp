

#include <iostream>
#include <fstream>

#include "clippy/clippy-eval.hpp"

#include <boost/json/src.hpp>

namespace bjsn = boost::json;

bjsn::value
parseFile(std::istream& inps)
{
  bjsn::stream_parser p;
  std::string         line;

  while (inps >> line)
  {
    bjsn::error_code ec;

    p.write(line.c_str(), line.size(), ec);

    if (ec) return nullptr;
  }

  bjsn::error_code ec;
  p.finish(ec);
  if (ec) return nullptr;

  return p.release();
}

bjsn::value
parseFile(const std::string& filename)
{
  std::ifstream is{filename};

  return parseFile(is);
}


int main()
{
  bjsn::value val = parseFile(std::cin);

  //~ std::cout << val << " --> " << std::endl;
  auto [ast, vars, hasComputed] = json_logic::translateNode(val);

  std::cerr << "Ast has " << vars.size() << " free variables"
            << (hasComputed ? ", and variables with computed names" : "")
            << "."
            << std::endl;

  json_logic::ValueExpr res     = json_logic::calculate(ast);

  std::cout << res << std::endl;
}
