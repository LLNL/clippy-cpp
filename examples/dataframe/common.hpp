#pragma once

#include <numeric>

namespace
{
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
}    
