

#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>

#include "clippy/clippy-eval.hpp"

#include <boost/json/src.hpp>

namespace bjsn = boost::json;

bjsn::value
parseStream(std::istream& inps)
{
  bjsn::stream_parser p;
  std::string         line;

  while (inps >> line)
  {
    bjsn::error_code ec;

    p.write(line.c_str(), line.size(), ec);

    if (ec) return nullptr;
  }

  bjsn::error_code ec;
  p.finish(ec);
  if (ec) return nullptr;

  return p.release();
}

bjsn::value
parseFile(const std::string& filename)
{
  std::ifstream is{filename};

  return parseStream(is);
}


template <class N, class T>
bool matchOpt1(const std::vector<std::string>& args, N& pos, std::string opt, T& fld)
{
  std::string arg(args.at(pos));

  if (arg.find(opt) != 0) return false;

  ++pos;
  fld = boost::lexical_cast<T>(args.at(pos));
  ++pos;
  return true;
}

template <class N, class Fn>
bool matchOpt1(const std::vector<std::string>& args, N& pos, std::string opt, Fn fn)
{
  std::string arg(args.at(pos));

  if (arg.find(opt) != 0) return false;

  ++pos;
  fn(args.at(pos));
  ++pos;
  return true;
}

template <class N, class Fn>
bool matchOpt0(const std::vector<std::string>& args, N& pos, std::string opt, Fn fn)
{
  std::string arg(args.at(pos));

  if (arg.find(opt) != 0) return false;

  fn();
  ++pos;
  return true;
}

template <class N>
bool unmatched(const std::vector<std::string>& args, N& pos)
{
  std::cerr << "unrecognized argument: " << args[pos] << std::endl;
  ++pos;
  return true;
}


int main(int argc, const char** argv)
{
  constexpr bool MATCH = false;

  bool verbose     = false;
  bool genExpected = false;

  int                        errorCode = 0;
  std::vector<std::string>   arguments(argv, argv+argc);
  size_t                     argn = 1;

  auto setVerbose = [&verbose]()     -> void { verbose     = true; };
  auto setResult  = [&genExpected]() -> void { genExpected = true; };

  while (argn < arguments.size())
  {
    MATCH
    || matchOpt0(arguments, argn, "-v",        setVerbose)
    || matchOpt0(arguments, argn, "--verbose", setVerbose)
    || matchOpt0(arguments, argn, "-r",        setResult)
    || matchOpt0(arguments, argn, "--result",  setResult)
    || unmatched(arguments, argn)
    ;
  }

  bjsn::value   all         = parseStream(std::cin);
  bjsn::object& allobj      = all.as_object();

  bjsn::value   rule        = allobj["rule"];
  const bool    hasData     = allobj.contains("data");
  bjsn::value   dat;
  const bool    hasExpected = allobj.contains("expected");

  if (hasData)
    dat = allobj["data"];
  else
    dat.emplace_object();

  try
  {
    json_logic::ValueExpr res     = json_logic::apply(rule, dat);

    if (verbose)
      std::cerr << res << std::endl;

    if (genExpected)
    {
      std::stringstream resStream;

      resStream << res;

      allobj["expected"] = parseStream(resStream);

      if (verbose) std::cerr << allobj["expected"] << std::endl;

      std::cout << allobj << std::endl;
    }
    else if (hasExpected)
    {
      std::stringstream expStream;
      std::stringstream resStream;

      expStream << allobj["expected"];
      resStream << res;
      errorCode = expStream.str() != resStream.str();

      if (verbose && errorCode)
        std::cerr << "test failed: "
                  << "\n  exp: " << expStream.str()
                  << "\n  got: " << resStream.str()
                  << std::endl;
    }
  }
  catch (const std::exception& ex)
  {
    if (verbose)
      std::cerr << "caught error: " << ex.what() << std::endl;

    errorCode = 1;
  }
  catch (...)
  {
    if (verbose)
      std::cerr << "caught unknown error" << std::endl;

    errorCode = 1;
  }

  if (verbose && errorCode)
    std::cerr << "errorCode: " << errorCode << std::endl;

  return errorCode;
}
