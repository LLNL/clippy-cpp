// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include "clippy/clippy.hpp"

#include "df-common.hpp"
#include "experimental/csv-io.hpp"

const std::string ARG_IMPORTED = "csvfile";
const std::string METHOD_NAME  = "importFile";

using DataConverter = std::function<xpr::dataframe_variant_t(const std::string& s)>;

template <class DfCellType>
struct ConverterFn
{
  ConverterFn(xpr::DataFrame* /* not needed */)
  {}

  xpr::dataframe_variant_t operator()(const std::string& s)
  {
    return boost::lexical_cast<DfCellType>(s);
  }
};

template <>
struct ConverterFn<xpr::string_t>
{
  xpr::dataframe_variant_t operator()(const std::string& s)
  {
    return df->persistent_string_std(s);
  }

  xpr::DataFrame* df;
};


template <class ColType>
bool push_conv_if(std::vector<DataConverter>& res, xpr::DataFrame& df, const xpr::ColumnDesc& coldesc)
{
  if (!coldesc.is<ColType>()) return false;

  res.push_back(ConverterFn<ColType>{&df});
  return true;
}

std::vector<DataConverter>
makeDataConverter(xpr::DataFrame& df)
{
  const std::vector<int>             colIndcs = allColumnIndices(df);
  const std::vector<xpr::ColumnDesc> colDescs = df.get_column_descriptors(colIndcs);

  std::vector<DataConverter>         res;

  for (const xpr::ColumnDesc& col : colDescs)
  {
    FirstMatch
    || push_conv_if<xpr::int_t>   (res, df, col)
    || push_conv_if<xpr::uint_t>  (res, df, col)
    || push_conv_if<xpr::real_t>  (res, df, col)
    || push_conv_if<xpr::string_t>(res, df, col)
    || fail("invalid Column Name");
  }

  return res;
}


std::string validateFilename(std::string filename)
{
  if (filename.size() == 0)
    fail("filename not provided");

  if (!boost::ends_with(boost::to_upper_copy(filename), ".CSV"))
    fail("filename does not end with \".csv\".");

  return filename;
}



int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Initializes a Dataframe object"};

  clip.member_of("Dataframe", "A dataframe");

  clip.add_required<std::string>( ARG_IMPORTED,
                                  "Imported File. Currently only CSV format is supported."
                                  "\n  The entries in the file need to follow the defined dataframe format."
                                );
  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");
  clip.add_required_state<std::string>(ST_DATAFRAME_NAME,  "Name of the dataframe object");

  if (clip.parse(argc, argv)) { return 0; }

  try
  {
    std::string                     filename = validateFilename(clip.get<std::string>(ARG_IMPORTED));
    std::string                     location = clip.get_state<std::string>(ST_METALL_LOCATION);
    std::string                     key = clip.get_state<std::string>(ST_DATAFRAME_NAME);
    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(false, location, key);
    xpr::DataFrame&                 df  = *dfp;
    const int                       numrows = df.rows();
    std::stringstream               msg;

    xpr::importCSV_variant(df, filename, makeDataConverter(df));
    msg << (df.rows()-numrows) << " records imported" << std::flush;

    clip.to_return(msg.str());
  }
  catch (const std::exception& err)
  {
    error_code = 1;
    clip.to_return(err.what());
  }

  return error_code;
}
