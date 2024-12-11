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

#include "clippy/selector.hpp"

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
  std::map<selector, std::variant<series<Vs>...>> data;
  std::map<selector, std::string> series_desc;

 public:
  // A series_proxy is a reference to a series in an mvmap.
  template <typename V>
  class series_proxy {
    selector id;
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
    series_proxy(selector id, series<V> &ser, mvmap<K, Vs...> &m)
        : id(std::move(id)), kti_r(m.kti), itk_r(m.itk), series_r(ser) {}

    series_proxy(selector id, const std::string &desc, series<V> &ser,
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

    std::pair<std::optional<std::pair<V, locator>>,
              std::optional<std::pair<V, locator>>>
    extrema() {
      V min = std::numeric_limits<V>::max();
      V max = std::numeric_limits<V>::min();
      bool found_min = false;
      bool found_max = false;
      locator min_loc;
      locator max_loc;
      for_all([&min, &max, &found_min, &found_max, &min_loc, &max_loc](
                  auto k, auto l, auto v) {
        if (v < min) {
          min = v;
          min_loc = l;
          found_min = true;
        }
        if (v > max) {
          max = v;
          max_loc = l;
          found_max = true;
        }
      });
      std::optional<std::pair<V, locator>> min_opt =
          found_min ? std::make_pair(min, min_loc) : std::nullopt;
      std::optional<std::pair<V, locator>> max_opt =
          found_max ? std::make_pair(max, max_loc) : std::nullopt;
      return std::make_pair(min_opt, max_opt);
    }

    std::map<V, size_t> histogram(size_t n_bins = 0) {
      std::map<V, size_t> hist;
      auto [min, max] = extrema();
      if (!min.has_value() || !max.has_value()) {
        return hist;
      }
      V min_val = min.value().first;
      V max_val = max.value().first;
      if (min_val == max_val) {
        hist[min_val] = series_r.size();
        return hist;
      }
      if (n_bins == 0) {
        n_bins = series_r.size();
      }
      V bin_width = (max_val - min_val) / n_bins;
      for_all([&hist, min_val, bin_width](auto /*unused*/, auto /*unused*/,
                                          auto v) {
        hist[min_val + (v - min_val) / bin_width]++;
      });
      return hist;
    }

  };  // end of series
  /////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////
  mvmap(const idx_to_key &itk, const key_to_idx &kti,
        const std::map<selector, std::variant<series<Vs>...>> &data)
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
    return {
        boost::json::value_to<idx_to_key>(obj.at("itk")),
        boost::json::value_to<key_to_idx>(obj.at("kti")),
        boost::json::value_to<std::map<selector, std::variant<series<Vs>...>>>(
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

  [[nodiscard]] std::vector<selector> list_series() const {
    return std::views::keys(data);
  }

  [[nodiscard]] bool has_series(const selector &id) const {
    return data.contains(id);
  }

  template <typename V>
  [[nodiscard]] bool has_series(const selector &id) const {
    return data.contains(id) && std::holds_alternative<series<V>>(data[id]);
  }

  bool contains(const K &k) { return kti.contains(k); }
  auto keys() const { return std::views::keys(kti); }

  // adds a new column (series) to the mvmap and returns true. If already
  // exists, return false
  template <typename V>
  std::optional<series_proxy<V>> add_series(const selector &sel,
                                            const std::string &desc = "") {
    if (has_series(sel)) {
      return std::nullopt;
    }
    data[sel] = series<V>{};
    series_desc[sel] = desc;
    return series_proxy(sel, desc, std::get<series<V>>(data[sel]), *this);
  }

  // copies an existing column (series) to a new (unmanifested) column and
  // returns true. If the new column already exists, or if the existing column
  // doesn't, return false.
  bool copy_series(const selector &from, const selector &to) {
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
  std::optional<series_proxy<V>> get_series(const selector &sel) {
    if (!has_series<V>(sel)) {
      // series doesn't exist or is of the wrong type.
      return std::nullopt;
    }
    return series_proxy<V>(sel, std::get<series<V>>(data[sel]), *this);
  }

  std::optional<series_proxy<variants>> get_variant_series(
      const selector &sel) {
    if (!has_series(sel)) {
      return std::nullopt;
    }
    return series_proxy<variants>(sel, data[sel], *this);
  }

  void drop_series(const selector &sel) {
    if (!has_series(sel)) {
      return;
    }
    data.erase(sel);
    series_desc.erase(sel);
  }

  mvmap::variants get_as_variant(const selector &sel, const locator &loc) {
    return data[sel][loc.loc];
  }

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
