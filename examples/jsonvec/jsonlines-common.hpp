#pragma once

#include <metall/metall.hpp>
#include <metall/container/vector.hpp>
#include <metall/container/experimental/json/json.hpp>

using json_value_type = metall::container::experimental::json::value<metall::manager::allocator_type<std::byte>>;
using vector_json_type = metall::container::vector<json_value_type, metall::manager::scoped_allocator_type<json_value_type>>;

namespace
{
  const std::string CLASS_NAME = "JsonLines";
  const std::string ST_METALL_LOCATION = "metall_location";
  const std::string ST_SELECTED = "selected";
  const std::string SELECTOR = "keys";
  // const std::string ST_JSONLINES_KEY  = "dataframe_key";
}

