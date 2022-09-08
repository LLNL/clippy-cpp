#pragma once

#include <numeric>
#include "experimental/dataframe.hpp"
#include "experimental/cxx-compat.hpp"

namespace xpr = experimental;

namespace
{
  //
  constexpr bool FirstMatch = false;

  // common constants
  const std::string ST_METALL_LOCATION = "metall_location";
  const std::string ST_DATAFRAME_NAME  = "dataframe_key";


  //
  // common convenience functions
  inline
  std::unique_ptr<experimental::DataFrame>
  makeDataFrame( bool create,
                 const std::string& persistent_location,
                 const std::string& persistent_key
               )
  {
    using DataFrame = experimental::DataFrame;

    DataFrame* res = create ? new DataFrame{metall::create_only_t{}, persistent_location.c_str(), persistent_key.c_str()}
                            : new DataFrame{metall::open_only_t{},   persistent_location.c_str(), persistent_key.c_str()}
                            ;

    assert(res);
    return std::unique_ptr<DataFrame>{res};
  }


  inline
  std::vector<int>
  allColumnIndices(const experimental::DataFrame& df)
  {
    const int        numcol = df.columns();
    std::vector<int> res(numcol);

    std::iota(res.begin(), res.end(), 0);
    return res;
  }

  CXX_NORETURN
  inline
  bool fail(std::string msg)
  {
    throw std::runtime_error{std::move(msg)};
  }
}


