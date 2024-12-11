#pragma once
#include <deque>
#include <ranges>

#include "boost/json.hpp"

class selector {
  std::deque<std::string> components;

 public:
  selector(std::string sel_str) {
    std::deque<std::string> comps;
    std::string::size_type start = 0;
    std::string::size_type end = sel_str.find('.');
    while (end != std::string::npos) {
      comps.push_back(sel_str.substr(start, end - start));
      start = end + 1;
      end = sel_str.find('.', start);
    }
    comps.push_back(sel_str.substr(start));
    components = comps;
  }

  std::string to_str() const {
    std::string result;
    for (const auto &comp : components) {
      result += comp + ".";
    }
    return result.substr(0, result.size() - 1);
  }

  bool headeq(std::string comp) const { return components[0] == comp; }
  std::string pop() {
    auto head = components[0];
    components.pop_front();
    return head;
  }
  selector tag_invoke(boost::json::value_to_tag<selector> /*unused*/,
                      const boost::json::value &v) {
    const auto &obj = v.as_object();
    auto sel = boost::json::value_to<std::string>(obj.at("selector"));
    return {sel};
  }
};