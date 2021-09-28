// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <set>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include <utility>
#include <boost/json/src.hpp>

#if __has_include(<mpi.h>)
#include <mpi.h>
#endif

namespace clippy {

class clippy {
 public:
  clippy(const std::string &&name, const std::string &&desc) {
    get_value(m_json_config, "method_name") = name;
    get_value(m_json_config, "desc") = desc;
  }

  ~clippy() {
    if (return_values) {
      int rank = 0;
#ifdef MPI_VERSION
      if (::MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
      }
#endif
      if (rank == 0) {
        std::cout << m_json_return << std::endl;
      }
    }
  }

  template <typename T>
  void add_required(const std::string &&name, const std::string &&desc) {
    add_required_validator<T>(name);
    size_t position = m_next_position++;
    get_value(m_json_config, "args", name, "desc") = desc;
    get_value(m_json_config, "args", name, "position") = position;
  }

  template <typename T>
  void add_optional(const std::string &&name, const std::string &&desc,
                    const T &default_val) {
    add_optional_validator<T>(name);
    get_value(m_json_config, "args", name, "desc") = desc;
    get_value(m_json_config, "args", name, "position") = -1;
    get_value(m_json_config, "args", name, "default_val") = default_val;
  }

  template <typename T>
  void returns(const std::string &&desc) {
    get_value(m_json_config, "returns", "desc") = desc;
  }

  template <typename T>
  void to_return(const T &value) {
    // if (detail::get_type_name<T>() !=
    //     m_json_config["returns"]["type"].get<std::string>()) {
    //   throw std::runtime_error("clippy::to_return(value):  Invalid type.");
    // }
    return_values = true;
    m_json_return = boost::json::value_from(value);
  }

  bool parse(int argc, char **argv) {
    const char *JSON_FLAG = "--clippy-help";
    const char *DRYRUN_FLAG = "--clippy-validate";
    if (argc == 2 && std::string(argv[1]) == JSON_FLAG) {
      std::cout << m_json_config;
      return true;
    }
    std::string buf;
    std::getline(std::cin, buf);
    m_json_input = boost::json::parse(buf);
    validate_json_input();

    if (argc == 2 && std::string(argv[1]) == DRYRUN_FLAG) { return true; }

    // Good to go for reals
    return false;
  }

  template <typename T>
  T get(const std::string &&name) {
    if (has_argument(name)) {  // if the argument exists
      return boost::json::value_to<T>(get_value(m_json_input, name));
    } else {  // it's an optional
      // std::cout << "optional argument found: " + name << std::endl;
      return boost::json::value_to<T>(get_value(m_json_config, "args", name, "default_val"));
    }
  }

  bool has_argument(const std::string &name) {
    return m_json_input.get_object().contains(name);
  }

 private:
  void validate_json_input() {
    for (auto &kv : m_input_validators) { kv.second(m_json_input); }
    // TODO: Warn/Check for unknown args
  }

  template <typename T>
  void add_optional_validator(const std::string &name) {
    if (m_input_validators.count(name) > 0) {
      std::stringstream ss;
      ss << "CLIPPy ERROR:   Cannot have duplicate argument names: " << name
         << "\n";
      throw std::runtime_error(ss.str());
    }
    m_input_validators[name] = [name](const boost::json::value &j) {
      if (!j.get_object().contains(name)) { return; }  // Optional, only eval if present
      try {
        boost::json::value_to<T>(get_value(j, name));
      } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "CLIPPy ERROR:  Optional argument " << name << ": \"" << e.what()
           << "\"\n";
        throw std::runtime_error(ss.str());
      }
    };
  }

  template <typename T>
  void add_required_validator(const std::string &name) {
    if (m_input_validators.count(name) > 0) {
      throw std::runtime_error("Clippy:: Cannot have duplicate argument names");
    }
    m_input_validators[name] = [name](const boost::json::value &j) {
      if (!j.get_object().contains(name)) {
        std::stringstream ss;
        ss << "CLIPPy ERROR:  Required argument " << name << " missing.\n";
        throw std::runtime_error(ss.str());
      }
      try {
        boost::json::value_to<T>(get_value(j, name));
      } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "CLIPPy ERROR:  Required argument " << name << ": \"" << e.what()
           << "\"\n";
        throw std::runtime_error(ss.str());
      }
    };
  }

  static boost::json::value &get_value(boost::json::value &value, const std::string &key) {
    if (!value.is_object()) {
      value.emplace_object();
    }
    return value.get_object()[key];
  }

  template <typename ...argts>
  static boost::json::value &get_value(boost::json::value &value,
                                              const std::string &key,
                                              const argts &... inner_keys) {
    if (!value.is_object()) {
      value.emplace_object();
    }
    return get_value(value.get_object()[key], inner_keys...);
  }

  static const boost::json::value &get_value(const boost::json::value &value, const std::string &key) {
    return value.get_object().at(key);
  }

  template <typename ...argts>
  static const boost::json::value &get_value(const boost::json::value &value,
                                             const std::string &key,
                                             const argts &... inner_keys) {
    return get_value(value.get_object().at(key), inner_keys...);
  }

  boost::json::value m_json_config;
  boost::json::value m_json_input;
  boost::json::value m_json_return;
  size_t m_next_position = 0;

  bool return_values = false;

  std::map<std::string, std::function<void(const boost::json::value &)>>
      m_input_validators;
};

}  // namespace clippy

namespace boost::json {
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv, const std::vector<std::pair<int, int>> &value) {
  auto &outer_array = jv.emplace_array();
  outer_array.resize(value.size());

  for (std::size_t i = 0; i < value.size(); ++i) {
    auto &inner_array = outer_array[i].emplace_array();
    inner_array.resize(2);
    inner_array[0] = value[i].first;
    inner_array[1] = value[i].second;
  }
}

std::vector<std::pair<int, int>> tag_invoke(boost::json::value_to_tag<std::vector<std::pair<int, int>>>,
                                            const boost::json::value &jv) {
  std::vector<std::pair<int, int>> value;

  auto &outer_array = jv.get_array();
  for (const auto &inner_value : outer_array) {
    const auto &inner_array = inner_value.get_array();
    value.emplace_back(std::make_pair(inner_array[0].as_int64(), inner_array[1].as_int64()));
  }

  return value;
}

void tag_invoke(boost::json::value_from_tag,
                boost::json::value &jv,
                const std::vector<std::pair<std::string, std::string>> &value) {
  auto &outer_array = jv.emplace_array();
  outer_array.resize(value.size());

  for (std::size_t i = 0; i < value.size(); ++i) {
    auto &inner_array = outer_array[i].emplace_array();
    inner_array.resize(2);
    inner_array[0] = value[i].first;
    inner_array[1] = value[i].second;
  }
}

std::vector<std::pair<std::string, std::string>> tag_invoke(boost::json::value_to_tag<std::vector<std::pair<std::string,
                                                                                                            std::string>>>,
                                                            const boost::json::value &jv) {
  std::vector<std::pair<std::string, std::string>> value;

  auto &outer_array = jv.get_array();
  for (const auto &inner_value : outer_array) {
    const auto &inner_array = inner_value.get_array();
    value.emplace_back(std::make_pair(std::string(inner_array[0].as_string().c_str()),
                                      std::string(inner_array[1].as_string().c_str())));
  }

  return value;
}
}