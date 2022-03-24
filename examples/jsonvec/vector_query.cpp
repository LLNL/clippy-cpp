// Copyright 2022 Lawrence Livermore National Security, LLC and other Metall Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: (Apache-2.0 OR MIT)

#include <iostream>
#include <string>
#include <metall/metall.hpp>
#include <metall/container/vector.hpp>
#include <metall/container/experimental/json/json.hpp>

#include "clippy/clippy-eval.hpp"

#include <boost/json/src.hpp>

namespace bjsn = boost::json;
namespace metjson = metall::container::experimental::json;
namespace jl = json_logic;

// Metall JSON container works like the other containers w.r.t. allocation, i.e.,
// it takes an allocator type in its template parameter and an allocator object in its constructors.
using json_value_type = metjson::value<metall::manager::allocator_type<std::byte>>;

// Need to use scoped_allocator as this is a multi-layer container.
using vector_json_type = metall::container::vector<json_value_type, metall::manager::scoped_allocator_type<json_value_type>>;


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


void createVector(const std::string& datafile)
{
  metall::manager manager(metall::create_only, "./test");
  auto *vec = manager.construct<vector_json_type>(metall::unique_instance)(manager.get_allocator());
  std::ifstream inp(datafile);
  std::string line;
  int i = 0;

  while (std::getline(inp, line))
  {
    if ((++i % 1024) == 0)
      std::cerr << i << std::endl;

    vec->emplace_back(metjson::parse(line, manager.get_allocator()));
  }
}

template <class T>
jl::ValueExpr toValueExpr_(const T& el)
{
  if (el.is_int64())  return jl::toValueExpr(el.as_int64());
  if (el.is_uint64()) return jl::toValueExpr(el.as_uint64());
  if (el.is_double()) return jl::toValueExpr(el.as_double());
  if (el.is_null()) return jl::toValueExpr(nullptr);

  assert(el.is_string()); // \todo array

  const auto& str = el.as_string();

  return jl::toValueExpr(bjsn::string(str.begin(), str.end()));
}

template <class T>
bool compiledQuery(const T& row)
{
  if (!row.is_object()) throw std::logic_error("Not a json::object");

  const auto& obj = row.as_object();

  {
    auto pos = obj.find("score");

    if (pos == obj.end()) return false;

    if (pos->value().is_int64())
    {
      if (pos->value().as_int64() <= 5)
        return false;
    }
    else if (pos->value().is_uint64())
    {
      if (pos->value().as_uint64() <= 5)
        return false;
    }
    else if (pos->value().is_double())
    {
      if (pos->value().as_double() <= 5)
        return false;
    }
    else
      return false;
  }

  {
    auto pos = obj.find("controversiality");

    if (pos == obj.end()) return false;

    if (pos->value().is_int64())
    {
      if (pos->value().as_int64() != 1)
        return false;
    }
    else if (pos->value().is_uint64())
    {
      if (pos->value().as_uint64() != 1)
        return false;
    }
    else if (pos->value().is_double())
    {
      if (pos->value().as_double() != 1)
        return false;
    }
    else
      return false;
  }

  return false;
}

void queryVector(const std::string& queryfile)
{
  bjsn::value logic = parseFile(queryfile);
  auto [ast, vars, hasComputedVariableNames] = jl::translateNode(logic);

  assert(!hasComputedVariableNames);

  metall::manager manager(metall::open_read_only, "./test");
  auto *vec = manager.find<vector_json_type>(metall::unique_instance).first;
  int matchingRows = 0;

  for (const auto& row : *vec)
  {
/*
    jl::ValueExpr val = jl::calculate( ast,
                                       [&row](const bjsn::string& colname, int) -> jl::ValueExpr
                                       {
                                         if (!row.is_object()) throw std::logic_error("Not a json::object");

                                         const auto& obj = row.as_object();
                                         std::string col(colname.begin(), colname.end());
                                         auto pos = obj.find(col);

                                         if (pos == obj.end()) return jl::toValueExpr(nullptr);

                                         return toValueExpr_(pos->value());
                                       }
                                     );

    if (toBool(val))
    {
      ++matchingRows;
      //~ metjson::pretty_print(std::cout, row);
    }
*/
    if (compiledQuery(row))
      ++matchingRows;
  }

  std::cerr << "found " << matchingRows << " matching rows." << std::endl;
}

int main(int argc, char** argv)
{
  int exitcode = 0;

  if (argc < 3)
  {
    std::cerr << "vector_query (create input_file | query json_logic_file)" << std::endl;
    return exitcode;
  }

  try
  {
    std::string cmd  = argv[1];
    std::string file = argv[2];

    if (cmd == "create")
    {
      createVector(file);
    }
    else if (cmd == "query")
    {
      queryVector(file);
    }
    else
    {
      std::cerr << "vector_query (create input_file | query json_logic_file)" << std::endl;
    }
  }
  catch (...)
  {
    std::cerr << "caught exception" << std::endl;
    exitcode = 1;
  }

  return exitcode;
}




