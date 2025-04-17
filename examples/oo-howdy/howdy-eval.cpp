

#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <jsonlogic/src.hpp>

#include "clippy/clippy.hpp"

namespace boostjsn = boost::json;
namespace jl = json_logic;

static const std::string className = "Greeter";
static const std::string methodName = "eval";
static const std::string expr = "expressions";
static const std::string selLetters = "letters";
static const std::string selFieldAll = "all";
static const std::string selFieldVowels = "vowels";
static const std::string selFieldConsonants = "consonants";
static const std::string stGreeting = "greeting";
static const std::string stGreeted = "greeted";
static const std::string stSelected = "selected";

int main(int argc, char** argv) {
  int error_code = 0;
  clippy::clippy clip{methodName, "Eval example"};

  clip.member_of(className, "Customizable Greeting Generator");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  try {
    using JsonExpression = std::vector<boost::json::object>;

    std::string greeting = clip.get_state<std::string>(stGreeting);
    std::string greeted = clip.get_state<std::string>(stGreeted);
    JsonExpression jsonExpression = clip.get_state<JsonExpression>(stSelected);
    std::string chars = greeting + " " + greeted;

    for (boost::json::object& jexp : jsonExpression) {
      auto [ast, vars, hasComputed] = jsonlogic::translateNode(jexp["rule"]);
      std::string tmp;

      tmp.swap(chars);

      for (char ch : tmp) {
        boostjsn::string elem(1, ch);
        jl::any_expr val = jl::calculate(
            ast, [&elem](const boostjsn::value&, int) -> jl::any_expr {
              return jl::toany_expr(elem);
            });
        if (jl::unpack_value<bool>(val)) chars += ch;
      }
    }

    clippy::object res;

    res.set_val("result", chars);

    /*
      auto [ast, vars, hasComputed] = jsonlogic::translateNode(val);

      std::cerr << "Ast has " << vars.size() << " free variables"
                << (hasComputed ? ", and variables with computed names" : "")
                << "."
                << std::endl;

      jsonlogic::any_expr res     = jsonlogic::calculate(*ast.get());
    */

    clip.to_return(chars);
  } catch (const std::exception& err) {
    clip.to_return(err.what());
    error_code = 1;
  }

  return error_code;
}
