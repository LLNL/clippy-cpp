#pragma once
#include "../include/mvmap.hpp"

class testdf {
  using variants = std::variant<std::string, double, int64_t, bool>;
  using idx_variants = std::variant<std::string, double, int64_t, bool>;
  using df_mvmap = mvmap::mvmap<uint64_t, std::string, double, int64_t, bool>;

  df_mvmap data;
  // std::optional<std::pair<std::string, df_mvmap::series_proxy<variants>>>
  //     index;

 public:
  testdf() = default;
  testdf(const df_mvmap &data) : data(data) {};

  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, const testdf &df) {
    std::optional<std::string> idx_name;
    // if (df.index.has_value()) {
    //   auto idx_pair = df.index.value();
    //   idx_name = idx_pair.first;
    // }

    v = {{"index_name", boost::json::value_from(idx_name)},
         {"data", boost::json::value_from(df.data)}};
  }

  friend testdf tag_invoke(boost::json::value_to_tag<testdf> /*unused*/,
                           const boost::json::value &v) {
    const auto &obj = v.as_object();
    df_mvmap data = boost::json::value_to<df_mvmap>(obj.at("data"));
    auto idx_name_o =
        boost::json::value_to<std::optional<std::string>>(obj.at("index_name"));

    testdf df{data};
    // if (idx_name_o.has_value()) {
    //   std::string idx_name = idx_name_o.value();
    //   if (data.series_is_bool(idx_name)) {
    //     df.set_index<bool>(idx_name);
    //   } else if (data.series_is_string(idx_name)) {
    //     df.set_index<std::string>(idx_name);
    //   } else if (data.series_is_int64_t(idx_name)) {
    //     df.set_index<int64_t>(idx_name);
    //   } else if (data.series_is_double(idx_name)) {
    //     df.set_index<double>(idx_name);
    //   } else {
    //     std::cerr << "Unknown index type; ignoring" << std::endl;
    //   }
    // }

    return df;
  }

  // bool is_index(const std::string &idx) {
  //   if (!index.has_value()) {
  //     return false;
  //   }
  //   return index.value().first == idx;
  // }

  // template <typename V>
  // bool set_index(const std::string &name) {
  //   if (data.has_series(name)) {
  //     return false;
  //   }
  //   auto ser_o = data.get_series<V>(name);
  //   if (!ser_o.has_value()) {
  //     return false;
  //   }
  //   auto ser = ser_o.value();
  //   index = std::make_pair(name, ser);
  //   // index.emplace(std::make_pair(name, ser));
  //   return true;
  // }

  // candidate function not viable: no known conversion from 'pair<typename
  // __unwrap_ref_decay<const string &>::type, typename
  // __unwrap_ref_decay<series_proxy<bool> &>::type>' (aka 'pair<std::string,
  // mvmap::mvmap<unsigned long long, bool, long long, double,
  // std::string>::series_proxy<bool>>') to 'nullopt_t' for 1st argument

  // bool drop_index() {
  //   if (!index.has_value()) {
  //     return false;
  //   }
  //   index = std::nullopt;
  //   return true;
  // }

  // bool drop_index(const std::string &name) {
  //   if (!is_index(name)) {
  //     return false;
  //   }
  //   index = std::nullopt;
  //   return true;
  // }

  template <typename V>
  std::optional<df_mvmap::series_proxy<V>> add_col(
      const std::string &name, const std::string &desc = "") {
    return data.add_series<V>(name, desc);
  }

  void drop_col(const std::string &name) {
    // drop_index(name);
    return data.drop_series(name);
  }

  template <typename F>
  void remove_if(F f) {
    return data.remove_if(f);
  }

  bool copy_col(const std::string &from, const std::string &to,
                const std::optional<std::string> &desc = std::nullopt) {
    return data.copy_series(from, to, desc);
  }

  std::vector<std::pair<std::string, std::string>> columns() {
    return data.list_series();
  }

  std::map<std::string, std::string> dtypes() {
    std::map<std::string, std::string> d{};
    for (auto [ser, _] : data.list_series()) {
      if (data.series_is_double(ser)) {
        d[ser] = "double";
      } else if (data.series_is_int64_t(ser)) {
        d[ser] = "int64_t";
      } else if (data.series_is_string(ser)) {
        d[ser] = "string";
      } else if (data.series_is_bool(ser)) {
        d[ser] = "bool";
      } else {
        d[ser] = "UNKNOWN";
      }
    }
    return d;
  }

  void add_row(const std::map<std::string, variants> &row) {
    return data.add_row(data.size(), row);
  }

  void add_rows(const std::vector<std::map<std::string, variants>> &rows) {
    for (const auto &row : rows) {
      add_row(row);
    }
  }

  void print() { data.print(); }

  void to_csv() {
    auto cols = columns();
    std::cout << "index,";
    for (auto col = cols.begin(); col != cols.end(); ++col) {
      bool last = col == std::prev(cols.end());
      std::cout << col->first;
      if (!last) {
        std::cout << ",";
      }
    }
    std::cout << std::endl;

    auto ks = data.keys();

    for (const auto &k : ks) {
      std::cout << k << ",";
      for (auto col = cols.begin(); col != cols.end(); ++col) {
        auto d_opt = data.get_as_variant(col->first, k);
        if (!d_opt.has_value()) {
          continue;
        }
        auto d = d_opt.value();
        bool last = col == std::prev(cols.end());
        std::visit(
            [&d, last](auto &&arg) {
              std::cout << arg;
              if (!last) {
                std::cout << ",";
              }
            },
            d);
      }
      std::cout << std::endl;
    }
  }
  void describe() { std::cout << "TestDF " << std::endl; }
};
