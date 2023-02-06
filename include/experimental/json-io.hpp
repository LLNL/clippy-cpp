

#pragma once

#include <stdexcept>
#include <vector>

#include <boost/json/src.hpp>

#include "cxx-compat.hpp"
#include "dataframe.hpp"

// use clippy-eval's conversion functions
#include <clippy/clippy-eval.hpp>


namespace experimental
{
  std::string OTHER_COLUMN = "__other_column%%";

namespace
{
  using ColumnID = std::optional<std::size_t>;

  ColumnID
  findColumn(const std::vector<std::string>& colnames, boost::string_view colname)
  {
    using iterator = std::vector<std::string>::const_iterator;

    iterator beg = colnames.begin();
    iterator lim = colnames.end();
    iterator pos = std::find(beg, lim, colname);

    return pos != lim ? ColumnID{std::distance(beg, pos)} : ColumnID{};
  }

  template <class T>
  T extractValue(const boost::json::value& val, T&& tag)
  {
    if (const boost::json::string* s = val.if_string())
      return json_logic::toConcreteValue(*s, tag);

    if (const std::int64_t* i = val.if_int64())
      return json_logic::toConcreteValue(*i, tag);

    if (const std::uint64_t* u = val.if_uint64())
      return json_logic::toConcreteValue(*u, tag);

    if (const double* d = val.if_double())
      return json_logic::toConcreteValue(*d, tag);

    if (val.is_null())
      return json_logic::toConcreteValue(std::nullptr_t{}, tag);

    throw std::logic_error("unsupported json value conversion.");
  }

  template <class T>
  T extractValue(json_logic::ValueExpr&& val, T&& tag)
  {
    return json_logic::unpackValue<T>(std::move(val));
  }


  void storeAtColumn( DataFrame& frame,
                      const ColumnVariant& coldesc,
                      dataframe_variant_t& cell,
                      const boost::json::value& val
                    )
  {
    const std::string& celltype = coldesc.type_name();

    if (celltype == string_type_str)
      cell = frame.persistent_string_std(extractValue(val, boost::json::string{}));
    else if (celltype == int_type_str)
      cell = extractValue(val, std::int64_t{});
    else if (celltype == uint_type_str)
      cell = extractValue(val, std::uint64_t{});
    else if (celltype == real_type_str)
      cell = extractValue(val, double{});
    else throw std::logic_error("unknown column type");
  }

  std::vector<int> allColumns(const DataFrame& frame)
  {
    std::vector<int> res(frame.columns());

    std::iota(res.begin(), res.end(), 0);
    return res;
  }

  auto toJson(const dataframe_variant_t& el) -> boost::json::value
  {
    if (const string_t* s = std::get_if<string_t>(&el))
      return boost::json::value{ boost::string_view(&*s->begin(), s->size()) };

    if (const int_t* i = std::get_if<int_t>(&el))
      return boost::json::value{*i};

    if (const real_t* r = std::get_if<real_t>(&el))
      return boost::json::value{*r};

    if (const uint_t* u = std::get_if<uint_t>(&el))
      return boost::json::value{*u};

    CXX_UNLIKELY;
    return boost::json::value{};
  }

  void injectValue(boost::json::object& o, const std::string& colname, const dataframe_variant_t& el)
  {
    if (std::get_if<notavail_t>(&el))
    {
      CXX_UNLIKELY;
      return;
    }

    o[colname] = toJson(el);
  }
}

inline
void setCellValue(DataFrame& frame, const ColumnVariant& coldesc, std::int64_t row, json_logic::ValueExpr&& val)
{
  ColumnVariant::pointer_variant_t ptr = coldesc.at_variant(row);

  if (string_t** s = std::get_if<string_t*>(&ptr))
    **s = frame.persistent_string_std(extractValue(std::move(val), boost::json::string{}));
  else if (int_t** i = std::get_if<int_t*>(&ptr))
    **i = extractValue(std::move(val), std::int64_t{});
  else if (uint_t** u = std::get_if<uint_t*>(&ptr))
    **u = extractValue(std::move(val), std::uint64_t{});
  else if (real_t** r = std::get_if<real_t*>(&ptr))
    **r = extractValue(std::move(val), std::uint64_t{});
  else
    throw std::runtime_error{"unknown column type"};
}


void importJson(DataFrame& frame, const boost::json::value& val, bool useOtherColumn = true)
{
  const boost::json::object&       obj = val.as_object();
  std::vector<dataframe_variant_t> row(frame.columns(), notavail_t{});
  std::vector<ColumnVariant>       allColumns      = frame.get_column_variants();
  std::vector<std::string>         allColumnNames  = frame.get_column_names();
  boost::json::object              other;

  for (auto& el : obj)
  {
    if (ColumnID colid = findColumn(allColumnNames, el.key()))
    {
      CXX_LIKELY;
      storeAtColumn(frame, allColumns.at(*colid), row.at(*colid), el.value());
    }
    else if (useOtherColumn)
      other.insert_or_assign(el.key(), el.value());
    else
      throw std::logic_error("automatic sparse column creation not yet supported");
  }

  if (!other.empty())
  {
    ColumnID colid = findColumn(allColumnNames, OTHER_COLUMN);

    assert(colid);
    assert(allColumns.at(*colid).type_name() == string_type_str);

    std::stringstream out;

    out << other;
    row.at(*colid) = frame.persistent_string_std(out.str());
  }

  frame.add(std::move(row));
}

//~ auto cellToJson(const dataframe_variant_t& el) -> boost::json::value
//~ {
  //~ return toJson(el);
//~ }

auto exportJson(const DataFrame& frame, const std::vector<std::string>& sel, std::size_t rownum) -> boost::json::object
{
  boost::json::object              res;
  std::vector<int>                 colsel = sel.size() ? frame.get_index_list_std(sel)
                                                       : allColumns(frame);
  std::vector<dataframe_variant_t> row = frame.get_row_variant(rownum, colsel);
  std::vector<std::string>         allColumnNames  = frame.get_column_names();

  for (int idx : colsel)
  {
    const std::string& colname = allColumnNames.at(idx);

    if (idx >= 0)
    {
      CXX_LIKELY;
      injectValue(res, colname, row.at(idx));
    }
    else
    {
      throw std::logic_error("other column conversion not yet supported");
      // res[colname] = toJsonObject(std::get<std::string>(row.at(idx)));
    }
  }

  return res;
}

} // namespace experimental


