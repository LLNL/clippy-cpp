// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "df-common.hpp"

using ColumnDescription = std::pair<std::string, std::string>;

const std::string& type(const ColumnDescription& col) { return col.second; }
const std::string& name(const ColumnDescription& col) { return col.first; }

static const std::string METHOD_NAME = "__init__";
static const std::string ARG_COLUMN_DESC = "columns";

void appendColumn(xpr::DataFrame& df, const ColumnDescription& desc)
{
  static const std::string UNKNOWN_COLUMN_TYPE{"unknown column type: "};

  if (type(desc) == "uint")
    df.add_column_default_value(xpr::dense<xpr::uint_t>{0});
  else if (type(desc) == "int")
    df.add_column_default_value(xpr::dense<xpr::int_t>{0});
  else if (type(desc) == "real")
    df.add_column_default_value(xpr::dense<xpr::real_t>{0});
  else if (type(desc) == "string")
    df.add_column_default_value(xpr::dense<xpr::string_t>{df.persistent_string("")});
  else
    throw std::runtime_error{UNKNOWN_COLUMN_TYPE + name(desc)};

  df.name_last_column(df.persistent_string(name(desc).c_str()));
}

void appendColumns(xpr::DataFrame& df, const std::vector<ColumnDescription>& cols)
{
  for (const ColumnDescription& desc : cols)
    appendColumn(df, desc);
}


int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Initializes a Dataframe object"};

  clip.member_of("Dataframe", "A dataframe");

  clip.add_required<std::string>(ST_METALL_LOCATION, "Location of the Metall store");
  clip.add_required<std::string>(ST_DATAFRAME_NAME,  "Name of the dataframe object within the Metall store");

  // ARG_COLUMN_DESC is optional. If not present, the __init__ method assumes that we want to
  //   reopen an existing dataframe.
  clip.add_optional<std::vector<ColumnDescription>>( ARG_COLUMN_DESC,
                                                     "Column description (pair of string/string describing name and type of columns)."
                                                     "\n  Valid types in (string | int | uint | real)",
                                                     std::vector<ColumnDescription>{}
                                                   );

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  try
  {
    // try to create the object
    std::string                     location  = clip.get<std::string>(ST_METALL_LOCATION);
    std::string                     key       = clip.get<std::string>(ST_DATAFRAME_NAME);
    const bool                      createNew = clip.has_argument(ARG_COLUMN_DESC);
    std::unique_ptr<xpr::DataFrame> dfp       = makeDataFrame(createNew, location, key);
    xpr::DataFrame&                 df        = *dfp;

    if (createNew)
    {
      // add the specified columns to the dataframe object.
      std::vector<ColumnDescription> column_desc = clip.get<std::vector<ColumnDescription>>(ARG_COLUMN_DESC);

      appendColumns(df, column_desc);
    }

    // set the return values
    clip.set_state(ST_METALL_LOCATION, std::move(location));
    clip.set_state(ST_DATAFRAME_NAME,  std::move(key));
  }
  catch (const std::runtime_error& ex)
  {
    error_code = 1;
    clip.to_return(ex.what());
  }

  return error_code;
}


