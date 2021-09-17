

#pragma once

#include <utility>
//~ #include <cassert>
//~ #include <stdexcept>
//~ #include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include "dataframe.hpp"

namespace experimental
{

namespace
{
template <class EX = std::logic_error>
inline
void csv_assert(bool cond, std::string msg) // , std::source_location pos = std::source_location::current())
{
  if (cond) return;
  
  std::stringstream errmsg;
  
  errmsg << msg 
         //~ << " @ " << pos.file_name 
         //~ << ":" << pos.line
         ;
         
  throw EX{errmsg.str()};
}  
  
auto readStr(std::istream& stream, char sep = ',') -> std::string
{
  std::string el;

  getline(stream, el, sep);
  return el;
}

auto readStrLn(std::istream& stream) -> std::string
{
  return readStr(stream, '\n');
}

bool eof(std::istream& stream)
{
  std::string tmp;
  stream >> tmp;
  
  return tmp == "" && stream.eof();
}

template <class T>
auto read(std::istream& stream, char sep = ',') -> T
{
  std::stringstream ss(readStr(stream, sep));
  T                 res;
  
  ss >> res;
  csv_assert(eof(ss), "error reading CSV value @1");
  return res;
}

template <class... Fields>
auto readTuple(std::istream& stream) -> std::tuple<Fields...>
{
  return std::tuple<Fields...>{ read<Fields>(stream)... };
}

template <class T>
auto readLn(std::istream& stream) -> T
{
  return read<T>(stream, '\n');
}

template <class ElemType>
auto readTupleVariant( std::istream& stream, 
                       const std::vector<std::function<ElemType(const std::string&)> >& adapt
                     ) -> std::vector<ElemType>
{
  std::vector<ElemType> res;
  
  for (const std::function<ElemType(const std::string&)>& celladapter : adapt)
    res.emplace_back(celladapter(readStr(stream)));
    
  return res;
}


template <class T>
struct WriteWrapper
{
  const T& elem;
};

template <class T>
std::ostream& operator<<(std::ostream& os, WriteWrapper<T> ww)
{
  return os << ww.elem; 
}

template <class T>
std::ostream& operator<<(std::ostream& os, WriteWrapper<std::optional<T>> ww)
{
  if (ww.elem) os << *ww.elem; 
  
  return os;
} 


template <class T>
void writeElem(std::ostream& os, T&& elem, size_t col)
{
  if (col) os << ", ";
  
  os << WriteWrapper<T>{elem};
}


template <class... Field, size_t... I>
void writeLn(std::ostream& os, std::tuple<Field...>&& rec, std::index_sequence<I...>)
{
  (writeElem<Field>(os, std::move(std::get<I>(rec)), I), ...);
  
  os << std::endl;
}

} // namespace anonymous

template<class... Fields>
auto importCSV(DataFrame& frame, std::istream& is)
{
  for (;;) 
  {
    std::string line = readStrLn(is);
    
    // break on eof
    if (!is) break;
    
    std::stringstream linestream(line);
      
    frame.add(readTuple<Fields...>(linestream));
    csv_assert(eof(linestream), "error reading CSV value @2");
  }
}

template<class... Fields>
auto importCSV(DataFrame& frame, const std::string& filename) -> void
{
  std::ifstream istream{filename};
  
  importCSV<Fields...>(frame, istream);
}

template<class... Fields>
auto importCSV(DataFrame& frame, const std::string& filename, const std::tuple<Fields...>*) -> void
{
  importCSV<Fields...>(frame, filename);
}


template <class ElemType>
auto importCSV_variant( DataFrame& frame, 
                        std::ifstream& is, 
                        const std::vector<std::function<ElemType(const std::string&)> >& adapt
                      ) -> void
{
  for (;;) 
  {
    std::string line = readStrLn(is);
    
    // break on eof
    if (!is) break;
    
    std::stringstream linestream(line);
      
    frame.add_variant(readTupleVariant(linestream, adapt));
    csv_assert(eof(linestream), "error reading CSV value @3");
  }
}
  
template <class ElemType>
auto importCSV_variant( DataFrame& frame, 
                        const std::string& filename, 
                        const std::vector<std::function<ElemType(const std::string&)> >& adapt
                      ) -> void
{
  std::ifstream istream{filename};
  
  importCSV_variant(frame, istream, adapt);
}


template<class... Fields>
auto exportCSV(const DataFrame& frame, std::ostream& os, const std::tuple<Fields...>* tag) -> void
{
  const size_t rowlimit = frame.rows();
  
  for (size_t row = 0; row < rowlimit; ++row)
  {
    std::tuple<Fields...> rec = frame.get_row(row, tag);
    
    writeLn<Fields...>(os, std::move(rec), std::make_index_sequence<sizeof... (Fields)>());
  }
}

template<class... Fields>
auto exportCSV(const DataFrame& frame, const std::string& filename, const std::tuple<Fields...>* tag) -> void
{
  std::ofstream ostream{filename};
  
  exportCSV(frame, ostream, tag);
}


} // namespace ygm


