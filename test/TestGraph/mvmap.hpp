#pragma once
#include <boost/json.hpp>
// #include <boost/json/conversion.hpp>
#include <boost/json/src.hpp>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <variant>

namespace mvmap {
using index = uint64_t;
class locator {
  static const index INVALID_LOC = std::numeric_limits<index>::max();
  index loc;

  locator(index loc) : loc(loc) {};

 public:
  template <typename K, typename... Vs>
  friend class mvmap;
  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, locator l);
  friend locator tag_invoke(boost::json::value_to_tag<locator> /*unused*/,
                            const boost::json::value &v);
  locator() : loc(INVALID_LOC) {};
  [[nodiscard]] bool is_valid() const { return loc != INVALID_LOC; }
};
void tag_invoke(boost::json::value_from_tag /*unused*/, boost::json::value &v,
                locator l) {
  v = l.loc;
}

locator tag_invoke(boost::json::value_to_tag<locator> /*unused*/,
                   const boost::json::value &v) {
  return boost::json::value_to<index>(v);
}
template <typename K, typename... Vs>
class mvmap {
  template <typename T>
  using series = std::map<index, T>;
  using key_to_idx = std::map<K, index>;
  using idx_to_key = std::map<index, K>;
  using variants = std::variant<Vs...>;

  // A locator is an opaque handle to a key in a series.

  idx_to_key itk;
  key_to_idx kti;
  std::map<std::string, std::variant<series<Vs>...>> data;
  std::map<std::string, std::string> series_desc;

 public:
  // A series_proxy is a reference to a series in an mvmap.
  template <typename V>
  class series_proxy {
    std::string id;
    std::string_view desc;
    key_to_idx &kti_r;
    idx_to_key &itk_r;
    series<V> &series_r;

    using series_type = V;

    // returns true if there is an index assigned to a given key
    bool has_idx(K k) { return kti_r.count(k) > 0; }
    // returns true if there is a key assigned to a given locator
    bool has_key(locator l) { return itk_r.count(l) > 0; }

    // returns or creates the index for a key.
    index get_idx(K k) {
      if (!has_idx(k)) {
        index i{kti_r.size()};
        kti_r[k] = i;
        itk_r[i] = k;
        return i;
      }
      return kti_r[k];
    }

   public:
    series_proxy(std::string id, series<V> &ser, mvmap<K, Vs...> &m)
        : id(std::move(id)), kti_r(m.kti), itk_r(m.itk), series_r(ser) {}

    series_proxy(std::string id, const std::string &desc, series<V> &ser,
                 mvmap<K, Vs...> &m)
        : id(std::move(id)),
          desc(desc),
          kti_r(m.kti),
          itk_r(m.itk),
          series_r(ser) {}

    V &operator[](K k) { return series_r[get_idx(k)]; }
    const V &operator[](K k) const { return series_r[get_idx(k)]; }

    // this assumes the key exists.
    V &operator[](locator l) { return series_r[l.loc]; }
    const V &operator[](locator l) const { return series_r[l.loc]; }

    std::optional<std::reference_wrapper<V>> at(locator l) {
      if (!has_key(l)) {
        return {};
      }
      return series_r[l.loc];
    };
    std::optional<std::reference_wrapper<const V>> at(locator l) const {
      if (!has_key(l)) {
        return {};
      }
      return series_r[l.loc];
    };

    std::optional<std::reference_wrapper<V>> at(K k) {
      if (!has_idx(k)) {
        return {};
      }
      return series_r[get_idx(k)];
    };

    std::optional<std::reference_wrapper<const V>> at(K k) const {
      if (!has_idx(k)) {
        return {};
      }
      return series_r[get_idx(k)];
    };

    // this will create the key/index if it doesn't exist.
    locator get_loc(K k) { return locator(get_idx(k)); }

    // F takes (K key, locator, V value)
    template <typename F>
    void for_all(F f) {
      for (auto el : series_r) {
        f(itk_r[el.first], locator(el.first), el.second);
      }
    };

    template <typename F>
    void for_all(F f) const {
      for (auto el : series_r) {
        f(itk_r[el.first], locator(el.first), el.second);
      }
    };

    // F takes (K key, locator, V value)
    template <typename F>
    void remove_if(F f) {
      auto indices_to_delete = std::vector<index>{};
      for (auto el : series_r) {
        if (f(itk_r[el.first], locator(el.first), el.second)) {
          indices_to_delete.emplace_back(el.first);
        }
      }

      for (auto ltd : indices_to_delete) {
        erase(locator(ltd));
      }
    };

    void erase(const locator &l) {
      auto i = l.loc;
      kti_r.erase(itk_r[i]);
      itk_r.erase(i);
      series_r.erase(i);
    }

    // if the key doesn't exist, do nothing.
    void erase(const K &k) {
      if (!has_idx(k)) {
        return;
      }
      auto i = kti_r[k];
      erase(locator(i));
    }

    // this returns the key for a given locator in a series, or nullopt if the
    // locator is invalid.
    std::optional<std::reference_wrapper<const K>> get_key(
        const locator &l) const {
      if (!has_idx(l.loc)) {
        return {};
      }
      return itk_r[l.loc];
    }
  };  // end of series
  /////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////
  mvmap(const idx_to_key &itk, const key_to_idx &kti,
        const std::map<std::string, std::variant<series<Vs>...>> &data)
      : itk(itk), kti(kti), data(data) {}

  mvmap() = default;
  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, const mvmap<K, Vs...> &m) {
    v = {{"itk", boost::json::value_from(m.itk)},
         {"kti", boost::json::value_from(m.kti)},
         {"data", boost::json::value_from(m.data)}};
  }

  friend mvmap<K, Vs...> tag_invoke(
      boost::json::value_to_tag<mvmap<K, Vs...>> /*unused*/,
      const boost::json::value &v) {
    const auto &obj = v.as_object();
    using index = uint64_t;
    // template <typename T> using series = std::map<index, T>;
    using key_to_idx = std::map<K, index>;
    using idx_to_key = std::map<index, K>;
    return {boost::json::value_to<idx_to_key>(obj.at("itk")),
            boost::json::value_to<key_to_idx>(obj.at("kti")),
            boost::json::value_to<
                std::map<std::string, std::variant<series<Vs>...>>>(
                obj.at("data"))};
  }

  [[nodiscard]] size_t size() const { return kti.size(); }
  bool add_key(const K &k) {
    if (kti.count(k) > 0) {
      return false;
    }
    auto i = kti.size();
    kti[k] = i;
    itk[i] = k;
    return true;
  }

  [[nodiscard]] std::vector<std::string_view> list_series() const {
    return std::views::keys(data);
  }

  [[nodiscard]] bool has_series(const std::string &id) const {
    return data.contains(id);
  }

  template <typename V>
  [[nodiscard]] bool has_series(const std::string &id) const {
    return data.contains(id) && std::holds_alternative<series<V>>(data[id]);
  }

  bool contains(const K &k) { return kti.contains(k); }
  auto keys() const { return std::views::keys(kti); }

  // adds a new column (series) to the mvmap and returns true. If already
  // exists, return false
  template <typename V>
  std::optional<series_proxy<V>> add_series(const std::string &name,
                                            const std::string &desc = "") {
    if (has_series(name)) {
      return std::nullopt;
    }
    data[name] = series<V>{};
    series_desc[name] = desc;
    return series_proxy(name, desc, std::get<series<V>>(data[name]), *this);
  }

  // copies an existing column (series) to a new column and returns true. If the
  // new column already exists, or if the existing column doesn't, return false.
  bool copy_series(const std::string &from, const std::string &to) {
    if (has_series(to) || !has_series(from)) {
      std::cerr << "copy_series failed from " << from << " to " << to
                << std::endl;
      return false;
    }
    std::cerr << "copying series from " << from << " to " << to << std::endl;
    data[to] = data[from];
    return true;
  }

  template <typename V>
  std::optional<series_proxy<V>> get_series(const std::string &name) {
    if (!has_series<V>(name)) {
      // series doesn't exist or is of the wrong type.
      return std::nullopt;
    }
    return series_proxy<V>(name, std::get<series<V>>(data[name]), *this);
  }

  std::optional<series_proxy<variants>> get_variant_series(
      const std::string &name) {
    if (!has_series(name)) {
      return std::nullopt;
    }
    return series_proxy<variants>(name, data[name], *this);
  }

  void drop_series(const std::string &name) {
    if (!has_series(name)) {
      return;
    }
    data.erase(name);
    series_desc.erase(name);
  }

  mvmap::variants get_as_variant(const std::string &name, const locator &loc) {
    return data[name][loc.loc];
  }

  // returns a series_proxy for the given string. If the series doesn't exist,
  // create it.
  // template <typename V>
  // series_proxy<V> get_or_create_series(const std::string &id,
  //                                      const std::string &desc = "") {
  //   if (data.count(id) == 0) {
  //     add_series<V>(id, desc);
  //   } else {
  //     if (!std::holds_alternative<series<V>>(data[id])) {
  //       throw std::runtime_error(
  //           "series id already exists with different type");
  //     }
  //   }
  //   return series_proxy<V>(id, desc, std::get<series<V>>(data[id]), *this);
  // }

  // F is a function that takes a key and a locator.
  // Users will need to close over series_proxies that they want to use.
  template <typename F>
  void for_all(F f) {
    for (auto &idx : kti) {
      f(idx.first, locator(idx.second));
    }
  }

  template <typename F>
  void remove_if(F f) {
    std::vector<index> indices_to_delete;
    for (auto &idx : kti) {
      if (f(idx.first, locator(idx.second))) {
        indices_to_delete.emplace_back(idx.second);
      }
    }

    for (auto &idx : indices_to_delete) {
      kti.erase(itk[idx]);
      itk.erase(idx);
      for (auto &id_ser : data) {
        std::visit([&idx](auto &ser) { ser.erase(idx); }, id_ser.second);
      }
    }
  }
};
};  // namespace mvmap
