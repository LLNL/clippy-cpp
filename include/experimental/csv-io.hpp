

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
  
#if OBSOLETE_CODE
auto readStr2(std::istream& stream, char sep = ',') -> std::string
{
  std::string el;

  getline(stream, el, sep);
  return el;
}
#endif /* OBSOLETE_CODE */

void processQuotedChar(std::istream& stream, std::vector<char>& buf, bool inclQuotes)
{
  if (inclQuotes) buf.push_back('"');

  while (true)
  {
    char ch = stream.get();

    if (ch == '"')
    {
      if (inclQuotes) buf.push_back('"');

      if (stream.peek() != '"')
        return;

      ch = stream.get();
}

    buf.push_back(ch);
  }
}

auto readStr(std::istream& stream, char sep = ',', bool inclQuotes = false) -> std::string
{
  std::vector<char> buf;
  char              ch = '\0';

  while (!stream.eof() && ((ch = stream.get()) != sep))
  {
    if (ch == '"')
      processQuotedChar(stream, buf, inclQuotes);
    else
      buf.push_back(ch);
  }

  return std::string{buf.begin(), buf.end()};
}

auto readStrLn(std::istream& stream, bool inclQuotes = false) -> std::string
{
  return readStr(stream, '\n', inclQuotes);
}

bool assertEof(std::istream& stream)
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
  csv_assert(assertEof(ss), "error reading CSV value @1");
  return res;
}

template <class T>
auto read_rec(std::istream& stream, T res, char sep = ',') -> T
{
  std::stringstream ss(readStr(stream, sep));

  ss >> res;
  csv_assert(assertEof(ss), "error reading CSV value @1");
  return res;
}

template <class T>
auto read_rec(std::istream& stream, experimental::string_t res, char sep = ',') -> T
{
  std::string inp = readStr(stream, sep);

  res.clear();
  res.append(inp.data(), inp.size());
  return res;
}

template <class... Fields>
auto readTuple(std::istream& stream) -> std::tuple<Fields...>
{
  return std::tuple<Fields...>{ read<Fields>(stream)... };
}

template <class... Fields, size_t... I>
auto readTuple_rec(std::istream& stream, std::tuple<Fields...> sample, std::index_sequence<I...>)
     -> std::tuple<Fields...>
{
  return std::tuple<Fields...>{ read_rec<Fields>(stream, std::move(std::get<I>(sample)))... };
}

template <class... Fields>
auto readTuple_rec(std::istream& stream, std::tuple<Fields...> sample) -> std::tuple<Fields...>
{
  return readTuple_rec(stream, std::move(sample), std::make_index_sequence<sizeof... (Fields)>());
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
    std::string line = readStrLn(is, true);
    
    // break on eof
    if (!is) break;
    
    std::stringstream linestream(line);
      
    frame.add(readTuple<Fields...>(linestream));
    csv_assert(assertEof(linestream), "error reading CSV value @2");
  }
}

template<class... Fields>
auto importCSV_rec(DataFrame& frame, std::istream& is, std::tuple<Fields...> sample)
{
  for (;;)
  {
    std::string line = readStrLn(is, true);

    // break on eof
    if (!is) break;

    std::stringstream linestream(line);

    frame.add(readTuple_rec(linestream, sample));
    csv_assert(assertEof(linestream), "error reading CSV value @2");
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


template<class... Fields>
auto importCSV_rec(DataFrame& frame, const std::string& filename, std::tuple<Fields...> sample) -> void
{
  std::ifstream istream{filename};

  importCSV_rec<Fields...>(frame, istream, std::move(sample));
}


template <class ElemType>
auto importCSV_variant( DataFrame& frame, 
                        std::ifstream& is, 
                        const std::vector<std::function<ElemType(const std::string&)> >& adapt
                      ) -> void
{
  for (;;) 
  {
    std::string line = readStrLn(is, true);
    
    // break on eof
    if (!is) break;
    
    std::stringstream linestream(line);
      
    frame.add_variant(readTupleVariant(linestream, adapt));
    csv_assert(assertEof(linestream), "error reading CSV value @3");
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


