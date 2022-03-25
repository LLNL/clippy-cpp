

#include <iostream>
#include <fstream>

#include <boost/json.hpp>
#include "clippy/clippy.hpp"
#include "clippy/clippy-eval.hpp"
#include "jsonlines-common.hpp"

namespace bjsn = boost::json;
namespace jl   = json_logic;

static const std::string methodName = "eval";

jl::ValueExpr toValueExpr(const json_value_type& el)
{
  if (el.is_int64())  return jl::toValueExpr(el.as_int64());
  if (el.is_uint64()) return jl::toValueExpr(el.as_uint64());
  if (el.is_double()) return jl::toValueExpr(el.as_double());
  if (el.is_null())   return jl::toValueExpr(nullptr);

  assert(el.is_string()); // \todo array

  const auto& str = el.as_string();

  return jl::toValueExpr(bjsn::string(str.begin(), str.end()));
}

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{methodName, "Eval example"};

  clip.member_of(CLASS_NAME, "A JsonLines class");

  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");
  clip.add_required_state<std::vector<bjsn::object>>(ST_SELECTED, "Selection expression");

  if (clip.parse(argc, argv)) { return 0; }

  try
  {
    using JsonExpression = std::vector<bjsn::object>;

    std::string              location = clip.get_state<std::string>(ST_METALL_LOCATION);
    //~ std::string             key = clip.get_state<std::string>(ST_JSONLINES_KEY);
    JsonExpression           jsonExpression = clip.get_state<JsonExpression>(ST_SELECTED);
    metall::manager          manager(metall::open_only, location.c_str());
    vector_json_type*        vec = manager.find<vector_json_type>(metall::unique_instance).first;

    if (vec == nullptr)
      throw std::runtime_error("Unable to open JsonObject");

    std::vector<jl::AnyExpr> queries;

    // prepare AST
    for (bjsn::object& jexp : jsonExpression)
    {
      auto [ast, vars, hasComputedVarNames] = json_logic::translateNode(jexp["rule"]);

      if (hasComputedVarNames) throw std::runtime_error("unable to work with computed variable names");

      // check that all free variables are prefixed with SELECTED
      for (const bjsn::string& varname : vars)
      {
        if (varname.rfind(SELECTOR, 0) != 0) throw std::logic_error("unknown selector");
        if (varname.find('.') != SELECTOR.size()) throw std::logic_error("unknown selector.");
      }

      queries.emplace_back(std::move(ast));
    }

    std::vector<int>     selectedRows;
    int                  rownum = 0;

    for (const auto& row : (*vec))
    {
      if (!row.is_object()) throw std::logic_error("Not a json::object");

      const auto& rowobj = row.as_object();
      const int   selLen = (SELECTOR.size() + 1);
      auto varLookup = [&rowobj,selLen](const bjsn::string& colname, int) -> jl::ValueExpr
                       {
                         std::string_view col{colname.begin() + selLen, colname.size() - selLen};
                         auto             pos = rowobj.find(col);

                         if (pos == rowobj.end()) return jl::toValueExpr(nullptr);

                         return toValueExpr(pos->value());
                       };

      auto rowPredicate = [varLookup](jl::AnyExpr& query) -> bool
                          {
                            return !toBool(jl::calculate(query, varLookup));
                          };

      const std::vector<jl::AnyExpr>::iterator lim = queries.end();
      const std::vector<jl::AnyExpr>::iterator pos = std::find_if(queries.begin(), lim, rowPredicate);

      if (pos == lim) selectedRows.push_back(rownum);

      ++rownum;
    }

    std::stringstream msg;

    msg << "Selected " << selectedRows.size() << " rows." << std::flush;
    clip.to_return(msg.str());
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;
  }

  return error_code;
}



