
#pragma once

namespace json_logic
{
  namespace json = boost::json;

  using JsonExpr = json::value;

  struct Expr {};

  // comparison
  struct Eq        : Expr {};
  struct StrictEq  : Expr {};
  struct Neq       : Expr {};
  struct StrictNeq : Expr {};
  struct Less      : Expr {};
  struct Greater   : Expr {};
  struct Leq       : Expr {};
  struct Geq       : Expr {};

  // logical operators
  struct And       : Expr {}
  struct Or        : Expr {}
  struct Not       : Expr {}
  struct NotNot    : Expr {}
  struct If        : Expr {}

  // arithmetics
  struct Add       : Expr {};
  struct Sub       : Expr {};
  struct Mul       : Expr {};
  struct Div       : Expr {};
  struct Mod       : Expr {};
  struct Min       : Expr {};
  struct Max       : Expr {};

  // array
  struct Map       : Expr {};
  struct Reduce    : Expr {};
  struct Filter    : Expr {};
  struct All       : Expr {};
  struct None      : Expr {};
  struct Some      : Expr {};
  struct Merge     : Expr {};

  // string operations
  struct Cat       : Expr {};
  struct Substr    : Expr {};

  // string and array operation
  struct In        : Expr {};

  // variable
  struct Var       : Expr {};

  // error node
  struct Error     : Expr {};

  // logger
  struct Log       : Expr {};

/*
  // types
  struct Type {};
  struct Int     : Type {};
  struct Real    : Type {};
  struct String  : Type {};
*/

  // Visitor
  struct Visitor
  {
    virtual void visit(JsonExpr&, const Expr&)         = 0; // error
    virtual void visit(JsonExpr&, const Eq&)           = 0;
    virtual void visit(JsonExpr&, const StrictEq&)     = 0;
    virtual void visit(JsonExpr&, const Neq&)          = 0;
    virtual void visit(JsonExpr&, const StrictNeq&)    = 0;
    virtual void visit(JsonExpr&, const Less&)         = 0;
    virtual void visit(JsonExpr&, const Greater&)      = 0;
    virtual void visit(JsonExpr&, const Leq&)          = 0;
    virtual void visit(JsonExpr&, const Geq&)          = 0;
    virtual void visit(JsonExpr&, const And&)          = 0;
    virtual void visit(JsonExpr&, const Or&)           = 0;
    virtual void visit(JsonExpr&, const Not&)          = 0;
    virtual void visit(JsonExpr&, const NotNot&)       = 0;
    virtual void visit(JsonExpr&, const Add&)          = 0;
    virtual void visit(JsonExpr&, const Sub&)          = 0;
    virtual void visit(JsonExpr&, const Mul&)          = 0;
    virtual void visit(JsonExpr&, const Div&)          = 0;
    virtual void visit(JsonExpr&, const Mod&)          = 0;
    virtual void visit(JsonExpr&, const Min&)          = 0;
    virtual void visit(JsonExpr&, const Max&)          = 0;
    virtual void visit(JsonExpr&, const Map&)          = 0;
    virtual void visit(JsonExpr&, const Reduce&)       = 0;
    virtual void visit(JsonExpr&, const Filter&)       = 0;
    virtual void visit(JsonExpr&, const All&)          = 0;
    virtual void visit(JsonExpr&, const None&)         = 0;
    virtual void visit(JsonExpr&, const Some&)         = 0;
    virtual void visit(JsonExpr&, const Merge&)        = 0;
    virtual void visit(JsonExpr&, const Cat&)          = 0;
    virtual void visit(JsonExpr&, const Substr&)       = 0;
    virtual void visit(JsonExpr&, const In&)           = 0;
    virtual void visit(JsonExpr&, const Var&)          = 0;
    virtual void visit(JsonExpr&, const Log&)          = 0;
    virtual void visit(JsonExpr&, const Error&)        = 0;

    // values
    virtual void visit(JsonExpr&, std::int64_t)        = 0;
    virtual void visit(JsonExpr&, std::uint64_t)       = 0;
    virtual void visit(JsonExpr&, double)              = 0;
    virtual void visit(JsonExpr&, const json::array&)  = 0;
    virtual void visit(JsonExpr&, const json::string&) = 0;
  };

  struct FwdVisitor : Visitor
  {
    void visit(JsonExpr& n, const Expr&)         override {} // error
    void visit(JsonExpr& n, const Eq&)           override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const StrictEq&)     override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Neq&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const StrictNeq&)    override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Less&)         override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Greater&)      override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Leq&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Geq&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const And&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Or&)           override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Not&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const NotNot&)       override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Add&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Sub&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Mul&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Div&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Mod&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Min&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Max&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Map&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Reduce&)       override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Filter&)       override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const All&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const None&)         override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Some&)         override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Merge&)        override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Cat&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Substr&)       override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const In&)           override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Var&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Log&)          override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const Error&)        override { visit(n, Expr{}); }

    void visit(JsonExpr& n, std::int64_t)        override { visit(n, Expr{}); }
    void visit(JsonExpr& n, std::uint64_t)       override { visit(n, Expr{}); }
    void visit(JsonExpr& n, double)              override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const json::array&)  override { visit(n, Expr{}); }
    void visit(JsonExpr& n, const json::string&) override { visit(n, Expr{}); }
  };

  /// AST traversal function that calls v's visit methods in post-fix order
  /// (note, types are not visited)
  void traverseInSAttributeOrder(JsonExpr&, Visitor&);

  /// traverses the children of a node; does not traverse grandchildren
  void traverseChildren(JsonExpr&, Visitor&);

  namespace
  {
    template <class JsonExprT>
    void visit(json::Value& n, Visitor& v)
    {
      v.visit(n, JsonExprT{});
    }

    template <class JsonExprT, class Val>
    void visit(json::Value& n, Visitor& v, const Val& val)
    {
      v.visit(n, val);
    }

    void dispatch(json::Value& n, Visitor& v)
    {
      using DispatchTable = std::map<std::string, void(*)(JsonExpr&, Visitor&)>;
      using Iterator      = json::Object::iterator;

      static constexpr DispatchTable dt = { { "==",     visit<Eq> },
                                            { "===",    visit<StrictEq> },
                                            { "!=",     visit<Neq> },
                                            { "!==",    visit<StrictNeq> },
                                            { "if",     visit<If> },
                                            { "!",      visit<Not> },
                                            { "!!",     visit<NotNot> },
                                            { "or",     visit<Or> },
                                            { "and",    visit<And> },
                                            { ">",      visit<Greater> },
                                            { ">=",     visit<Geq> },
                                            { "<",      visit<Less> },
                                            { "<=",     visit<Leq> },
                                            { "max",    visit<Max> },
                                            { "min",    visit<Min> },
                                            { "+",      visit<Add> },
                                            { "-",      visit<Sub> },
                                            { "*",      visit<Mul> },
                                            { "/",      visit<Div> },
                                            { "%",      visit<Mod> },
                                            { "map",    visit<Map> },
                                            { "reduce", visit<Reduce> },
                                            { "filter", visit<Filter> },
                                            { "all",    visit<All> },
                                            { "none",   visit<None> },
                                            { "some",   visit<Some> },
                                            { "merge",  visit<Merge> },
                                            { "in",     visit<In> },
                                            { "cat",    visit<Cat> },
                                            { "log",    visit<Log> },
                                            { "var",    visit<Var> }
                                          };


      if (json::Object* obj = n.is_object())
      {
        Iterator           aa  = obj.begin();

        assert(aa == o.end());

        const std::string& op = aa->first();

        if (DispatchTable::const_iterator pos = dt.find(op))
        {
          pos->second(n, v);
        }
        else
        {
          visit<Error>(n, v);
        }
      }
      else if (std::int64_t* val = n.is_int64())
      {
        val.visit(n, *val);
      }
      else if (std::uint64_t* val = n.is_uint64())
      {
        val.visit(n, *val);
      }
      else if (double* val = n.is_double())
      {
        visit(n, v, *val);
      }
      else if (json::array* arr = n.is_array())
      {
        visit(n, v, *arr);
      }
      else if (json::string* str = n.is_string())
      {
        visit(n, v, *str);
      }
      else
      {
        visit<Error>(n, v);
      }
    }
  }

  void traverseChildren(JsonExpr& n, Visitor& n)
  {
    using Iterator = json::Object::iterator;

    json::Object& o  = n.as_object();
    Iterator      aa = o.begin();
    Iterator      zz = o.end();

    assert(aa == zz);
    json::Object& operands = aa->second;
    json::Array&  children = operands.as_array();

    for (json::Value& elem : children)
      dispatch(elem, v);
  }

  int operandCount(JsonExpr& n)
  {
    using Iterator = json::Object::iterator;

    json::Object& o  = n.as_object();
    Iterator      aa = o.begin();
    Iterator      zz = o.end();

    assert(aa == zz);
    json::Object& operands = aa->second;
    json::Array&  children = operands.as_array();

    return children.size();
  }

  namespace
  {
    struct SAttributeTraversal : Visitor
    {
        explicit
        SAttributeTraversal(Visitor& client)
        : sub(client)
        {}

        void visit(JsonExpr& n, const Eq&)           override;
        void visit(JsonExpr& n, const StrictEq&)     override;
        void visit(JsonExpr& n, const Neq&)          override;
        void visit(JsonExpr& n, const StrictNeq&)    override;
        void visit(JsonExpr& n, const Less&)         override;
        void visit(JsonExpr& n, const Greater&)      override;
        void visit(JsonExpr& n, const Leq&)          override;
        void visit(JsonExpr& n, const Geq&)          override;
        void visit(JsonExpr& n, const And&)          override;
        void visit(JsonExpr& n, const Or&)           override;
        void visit(JsonExpr& n, const Not&)          override;
        void visit(JsonExpr& n, const NotNot&)       override;
        void visit(JsonExpr& n, const Add&)          override;
        void visit(JsonExpr& n, const Sub&)          override;
        void visit(JsonExpr& n, const Mul&)          override;
        void visit(JsonExpr& n, const Div&)          override;
        void visit(JsonExpr& n, const Mod&)          override;
        void visit(JsonExpr& n, const Min&)          override;
        void visit(JsonExpr& n, const Max&)          override;
        void visit(JsonExpr& n, const Map&)          override;
        void visit(JsonExpr& n, const Reduce&)       override;
        void visit(JsonExpr& n, const Filter&)       override;
        void visit(JsonExpr& n, const All&)          override;
        void visit(JsonExpr& n, const None&)         override;
        void visit(JsonExpr& n, const Some&)         override;
        void visit(JsonExpr& n, const Merge&)        override;
        void visit(JsonExpr& n, const Cat&)          override;
        void visit(JsonExpr& n, const Substr&)       override;
        void visit(JsonExpr& n, const In&)           override;
        void visit(JsonExpr& n, const Var&)          override;
        void visit(JsonExpr& n, const Log&)          override;
        void visit(JsonExpr& n, const Error&)        override;
                                                             ;
        void visit(JsonExpr& n, std::int64_t)        override;
        void visit(JsonExpr& n, std::uint64_t)       override;
        void visit(JsonExpr& n, double)              override;
        void visit(JsonExpr& n, const json::array&)  override;
        void visit(JsonExpr& n, const json::string&) override;

      private:
        Visitor& sub;

        template <class JsonExprT>
        inline
        void _visit(JsonExpr& n, const JsonExprT& tag)
        {
          traverseChildren(n, *this);
          sub.visit(n, tag);
        }

        template <class ValueT>
        inline
        void _value(JsonExpr& n, const ValueT& val)
        {
          sub.visit(n, val);
        }
    };

    void SAttributeTraversal::visit(JsonExpr& n, const Eq& tag)         { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const StrictEq& tag)   { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Neq& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const StrictNeq& tag)  { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Less& tag)       { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Greater& tag)    { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Leq& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Geq& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const And& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Or& tag)         { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Not& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const NotNot& tag)     { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Add& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Sub& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Mul& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Div& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Mod& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Min& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Max& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Map& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Reduce& tag)     { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Filter& tag)     { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const All& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const None& tag)       { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Some& tag)       { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Merge& tag)      { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Cat& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Substr& tag)     { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const In& tag)         { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Var& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Log& tag)        { _visit(n, tag); }
    void SAttributeTraversal::visit(JsonExpr& n, const Error& tag)      { _visit(n, tag); }

    void SAttributeTraversal::visit(JsonExpr& n, std::int64_t v)        { _value(n, v); }
    void SAttributeTraversal::visit(JsonExpr& n, std::uint64_t v)       { _value(n, v); }
    void SAttributeTraversal::visit(JsonExpr& n, double v)              { _value(n, v); }
    void SAttributeTraversal::visit(JsonExpr& n, const json::array& v)  { _value(n, v); }
    void SAttributeTraversal::visit(JsonExpr& n, const json::string& v) { _value(n, v); }
  }

  struct Calculator : FwdVisitor
  {
      using VarAccess = std::function<JsonExpr(const std::string& name)>;

      explicit
      Calculator(VarAccess varAccess)
      : vars(std::move(varAccess)), evalstack()
      {}

      void visit(JsonExpr& n, const Eq&)           override;
      void visit(JsonExpr& n, const StrictEq&)     override;
      void visit(JsonExpr& n, const Neq&)          override;
      void visit(JsonExpr& n, const StrictNeq&)    override;
      void visit(JsonExpr& n, const Less&)         override;
      void visit(JsonExpr& n, const Greater&)      override;
      void visit(JsonExpr& n, const Leq&)          override;
      void visit(JsonExpr& n, const Geq&)          override;
      void visit(JsonExpr& n, const And&)          override;
      void visit(JsonExpr& n, const Or&)           override;
      void visit(JsonExpr& n, const Not&)          override;
      void visit(JsonExpr& n, const NotNot&)       override;
      void visit(JsonExpr& n, const Add&)          override;
      void visit(JsonExpr& n, const Sub&)          override;
      void visit(JsonExpr& n, const Mul&)          override;
      void visit(JsonExpr& n, const Div&)          override;
      void visit(JsonExpr& n, const Mod&)          override;
      void visit(JsonExpr& n, const Min&)          override;
      void visit(JsonExpr& n, const Max&)          override;
      void visit(JsonExpr& n, const Map&)          override;
      void visit(JsonExpr& n, const Reduce&)       override;
      void visit(JsonExpr& n, const Filter&)       override;
      void visit(JsonExpr& n, const All&)          override;
      void visit(JsonExpr& n, const None&)         override;
      void visit(JsonExpr& n, const Some&)         override;
      void visit(JsonExpr& n, const Merge&)        override;
      void visit(JsonExpr& n, const Cat&)          override;
      void visit(JsonExpr& n, const Substr&)       override;
      void visit(JsonExpr& n, const In&)           override;
      void visit(JsonExpr& n, const Var&)          override;
      void visit(JsonExpr& n, const Log&)          override;
      void visit(JsonExpr& n, const Error&)        override;
                                                           ;
      void visit(JsonExpr& n, std::int64_t)        override;
      void visit(JsonExpr& n, std::uint64_t)       override;
      void visit(JsonExpr& n, double)              override;
      void visit(JsonExpr& n, const json::array&)  override;
      void visit(JsonExpr& n, const json::string&) override;

    private:
      VarAccess             vars;
      std::vector<JsonExpr> evalstack;

      std::vector<JsonExpr>::iterator
      arg(int num = 0)
      {
        return evalstack.end() - num;
      }

      void convertTypes(JsonExpr& n);

      template <class BinaryPredicate>
      void comparePairWise(JsonExpr& n, BinaryPredicate pred)
      {
        int                                   num = operandCount(n);
        assert(num >= 2);

        std::vector<JsonExpr>::iterator       aa  = arg(num);
        const std::vector<JsonExpr>::iterator zz  = arg(1);
        bool                                  res = true;

        while (res && (aa != zz))
        {
          res = pred(*(aa), *(aa+1));
          ++aa;
        }

        pop(num);
        push(res);
      }

      template <class UnaryPredicate>
      void testEachElement(JsonExpr& n, bool init)
      {
        int                                   num = operandCount(n);
        assert(num >= 2);

        // \todo convert all types to bool

        std::vector<JsonExpr>::iterator       aa  = arg(num);
        const std::vector<JsonExpr>::iterator zz  = arg();
        bool                                  res = init;

        while ((res == init) && (aa != zz))
        {
          if (asBool(*aa) == !init) res = !init;

          ++aa;
        }

        pop(num);
        push(res);
      }
  };

  void Calculator::convertTypes(JsonExpr& n)
  {
    int num = operandCount(n);
    auto conversion = std::for_each(arg(operands), arg(), TypeMatic{});

    std::for_each(arg(operands), arg(), conv);
  }

  void Calculator::convertTypes(int num)
  {
    auto conversion = std::for_each(arg(operands), arg(), TypeMatic{});

    std::for_each(arg(operands), arg(), conv);
  }


  void Calculator::visit(JsonExpr& n, const Eq& tag)
  {
    convertTypes(n);
    visit(n, StrictEq{});
  }

  void Calculator::visit(JsonExpr& n, const StrictEq& tag)
  {
    comparePairWise(n, strictEqual);
  }

  void Calculator::visit(JsonExpr& n, const Neq& tag)
  {
    convertTypes(num);
    visit(n, StrictNeq{});
  }

  void Calculator::visit(JsonExpr& n, const StrictNeq& tag)
  {
    comparePairWise(n, strictNotEqual);
  }

  void Calculator::visit(JsonExpr& n, const Less& tag)
  {
    convertTypes(n); // needed?

    comparePairWise(n, lessThan);
  }

  void Calculator::visit(JsonExpr& n, const Greater& tag)
  {
    convertTypes(n); // needed?

    comparePairWise(n, greaterThan);
  }

  void Calculator::visit(JsonExpr& n, const Leq& tag)
  {
    convertTypes(n); // needed?

    comparePairWise(n, lessOrEqualThan);
  }

  void Calculator::visit(JsonExpr& n, const Geq& tag)
  {
    convertTypes(n); // needed?

    comparePairWise(n, greaterOrEqualThan);
  }

  void Calculator::visit(JsonExpr& n, const And& tag)
  {
    testEachElement(n, false, true);
  }

  void Calculator::visit(JsonExpr& n, const Or& tag)
  {
    testEachElement(n, true, false);
  }

  void Calculator::visit(JsonExpr& n, const Not& tag)
  {
    bool res = logicalNegate(*arg());

    pop(1);
    push(res);
  }

  void Calculator::visit(JsonExpr& n, const NotNot& tag)
  {
    bool res = !logicalNegate(*arg());

    pop(1);
    push(res);
  }

  void Calculator::visit(JsonExpr& n, const Add& tag)
  {
    reduce(n, arithAdd);
  }

  void Calculator::visit(JsonExpr& n, const Sub& tag)
  {
    reduce(n, arithSub);
  }

  void Calculator::visit(JsonExpr& n, const Mul& tag)
  {
    reduce(n, arithMul);
  }

  void Calculator::visit(JsonExpr& n, const Div& tag)
  {
    reduce(n, arithDiv);
  }

  void Calculator::visit(JsonExpr& n, const Mod& tag)
  {
    reduce(n, arithMod);
  }

  void Calculator::visit(JsonExpr& n, const Min& tag)
  {
    reduce(n, arithMin);
  }

  void Calculator::visit(JsonExpr& n, const Max& tag)
  {
    reduce(n, arithMax);
  }

  void Calculator::visit(JsonExpr& n, const Map& tag)
  {
  }

  void Calculator::visit(JsonExpr& n, const Reduce& tag)
  {

  }

  void Calculator::visit(JsonExpr& n, const Filter& tag)
  {
    //
  }

  void Calculator::visit(JsonExpr& n, const All& tag)

  void Calculator::visit(JsonExpr& n, const None& tag)

  void Calculator::visit(JsonExpr& n, const Some& tag)

  void Calculator::visit(JsonExpr& n, const Merge& tag)

  void Calculator::visit(JsonExpr& n, const Cat& tag)

  void Calculator::visit(JsonExpr& n, const Substr& tag)

  void Calculator::visit(JsonExpr& n, const In& tag)

  void Calculator::visit(JsonExpr& n, const Var& tag)

  void Calculator::visit(JsonExpr& n, const Log& tag)

  void Calculator::visit(JsonExpr& n, const Error& tag)


  void Calculator::visit(JsonExpr& n, std::int64_t v)        { _value(n, v); }
  void Calculator::visit(JsonExpr& n, std::uint64_t v)       { _value(n, v); }
  void Calculator::visit(JsonExpr& n, double v)              { _value(n, v); }
  void Calculator::visit(JsonExpr& n, const json::array& v)  { _value(n, v); }
  void Calculator::visit(JsonExpr& n, const json::string& v) { _value(n, v); }


  void traverseInSAttributeOrder(JsonExpr& n, Visitor& vis)
  {
    SAttributeTraversal trav{vis};

    dispatch(n, trav);
  }
}
