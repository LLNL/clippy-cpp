// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"
#include "jsonlines-common.hpp"


namespace boostjsn = boost::json;
namespace metaljsn = metall::container::experimental::json;

static const std::string methodName = "__getitem__";

static const std::string expr = "expressions";


int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{methodName, "Selector example"};

  clip.member_of(CLASS_NAME, "A JsonLines class");

  clip.add_required<std::vector<boost::json::object>>(expr, "Expression selection");
  clip.add_selector<std::string>(SELECTOR, "Row Selector");
  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");

  //~ clip.returns<std::vector<std::string>>("Expression result");

  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  try
  {
    using JsonExpression = std::vector<boost::json::object>;

    std::string    location = clip.get_state<std::string>(ST_METALL_LOCATION);
    //~ std::string      key = clip.get_state<std::string>(ST_JSONLINES_KEY);
    JsonExpression jsonExpression = clip.get<JsonExpression>(expr);

    clippy::object res;
    clippy::object clippy_type;
    clippy::object state;

    state.set_val(ST_METALL_LOCATION, std::move(location));
    //~ state.set_val(ST_JSONLINES_KEY,   std::move(key));
    state.set_val(ST_SELECTED,        std::move(jsonExpression));

    clippy_type.set_val("__class__", CLASS_NAME);
    clippy_type.set_json("state",    std::move(state));

    res.set_json("__clippy_type__",  std::move(clippy_type));
    clip.to_return(std::move(res));
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;
  }

  return error_code;
}


