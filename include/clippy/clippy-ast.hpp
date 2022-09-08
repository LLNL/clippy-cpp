
#pragma once

#include <exception>
#include <algorithm>
#include <string>
#include <limits>
#include <memory>

#include <boost/json.hpp>

#include <experimental/cxx-compat.hpp>

namespace json_logic
{
  constexpr bool DEBUG_OUTPUT    = true;

  namespace json = boost::json;

  using JsonExpr = json::value;

  template <class T>
  T& as(T& n) { return n; }

  template <class T>
  T& deref(T* p)
  {
    assert(p);
    return *p;
  }

  struct Visitor;

  // the root class
  struct Expr
  {
      Expr()          = default;
      virtual ~Expr() = default;

      virtual void accept(Visitor&) = 0;

    private:
      Expr(Expr&&)                 = delete;
      Expr(const Expr&)            = delete;
      Expr& operator=(Expr&&)      = delete;
      Expr& operator=(const Expr&) = delete;
  };

  //~ Expr::~Expr() {}

  //
  // foundation classes
  // \{

  using AnyExpr = std::unique_ptr<Expr>;

  struct Operator : Expr, private std::vector<AnyExpr>
  {
    using container_type = std::vector<AnyExpr>;

    using container_type::iterator;
    using container_type::const_iterator;
    using container_type::const_reverse_iterator;
    using container_type::reverse_iterator;
    using container_type::begin;
    using container_type::end;
    using container_type::rbegin;
    using container_type::rend;
    using container_type::crbegin;
    using container_type::crend;
    using container_type::back;
    using container_type::push_back;

    void set_operands(container_type&& opers)
    {
      this->swap(opers);
    }

    Expr& operand(int n) const
    {
      return deref(this->at(n).get());
    }

    virtual int num_evaluated_operands() const;
  };

  // defines operators that have an upper bound on how many
  //   arguments are evaluated.
  template <int MaxArity>
  struct OperatorN : Operator
  {
    enum { MAX_OPERANDS = MaxArity };

    int num_evaluated_operands() const final;
  };

  template <class T>
  struct Value : Expr
  {
      using value_type = T;

      explicit
      Value(T t)
      : val(std::move(t))
      {}

      T&       value()       { return val; }
      const T& value() const { return val; }

    private:
      T val;
  };

  // \}

  //
  // expression hierarchy
  // comparison

  // binary
  struct Eq        : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct StrictEq  : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct Neq       : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct StrictNeq : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  // binary or ternary
  struct Less      : OperatorN<3>
  {
    void accept(Visitor&) final;
  };

  struct Greater   : OperatorN<3>
  {
    void accept(Visitor&) final;
  };

  struct Leq       : OperatorN<3>
  {
    void accept(Visitor&) final;
  };

  struct Geq       : OperatorN<3>
  {
    void accept(Visitor&) final;
  };

  // logical operators

  // unary
  struct Not       : OperatorN<1>
  {
    void accept(Visitor&) final;
  };

  struct NotNot    : OperatorN<1>
  {
    void accept(Visitor&) final;
  };

  // n-ary
  struct And       : Operator
  {
    void accept(Visitor&) final;
  };

  struct Or        : Operator
  {
    void accept(Visitor&) final;
  };


  // control structure
  struct If        : Operator
  {
    void accept(Visitor&) final;
  };

  // arithmetics
  // n-ary
  struct Add       : Operator
  {
    void accept(Visitor&) final;
  };

  struct Mul       : Operator
  {
    void accept(Visitor&) final;
  };

  struct Min       : Operator
  {
    void accept(Visitor&) final;
  };

  struct Max       : Operator
  {
    void accept(Visitor&) final;
  };


  // binary
  struct Sub       : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct Div       : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct Mod       : OperatorN<2>
  {
    void accept(Visitor&) final;
  };


  // array
  struct Array     : Operator  // array is modeled as operator
  {
    void accept(Visitor&) final;
  };

  struct Map       : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct Reduce    : OperatorN<3>
  {
    void accept(Visitor&) final;
  };

  struct Filter    : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct All       : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct None      : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct Some      : OperatorN<2>
  {
    void accept(Visitor&) final;
  };

  struct Merge     : Operator
  {
    void accept(Visitor&) final;
  };


  // string operations
  struct Var       : OperatorN<1>
  {
      enum { computed = -1 };

      void accept(Visitor&) final;

      void num(int val)       { idx = val;  }
      int  num()        const { return idx; }

    private:
      int idx = computed;
  };

  struct Cat       : Operator
  {
    void accept(Visitor&) final;
  };

  struct Substr    : OperatorN<3>
  {
    void accept(Visitor&) final;
  };


  // string and array operation
  struct In        : Operator
  {
    void accept(Visitor&) final;
  };


  // values
  struct NullVal   : Expr
  {
    void accept(Visitor&) final;

    std::nullptr_t value() const { return nullptr; }
  };

  struct BoolVal   : Value<bool>
  {
    using base = Value<bool>;
    using base::base;

    void accept(Visitor&) final;
  };

  struct IntVal    : Value<std::int64_t>
  {
    using base = Value<std::int64_t>;
    using base::base;

    void accept(Visitor&) final;
  };

  struct UintVal   : Value<std::uint64_t>
  {
    using base = Value<std::uint64_t>;
    using base::base;

    void accept(Visitor&) final;
  };

  struct DoubleVal : Value<double>
  {
    using base = Value<double>;
    using base::base;

    void accept(Visitor&) final;
  };

  struct StringVal : Value<json::string>
  {
    using base = Value<json::string>;
    using base::base;

    void accept(Visitor&) final;
  };

  // logger
  struct Log       : OperatorN<1>
  {
    void accept(Visitor&) final;
  };

  // error node
  struct Error     : Expr
  {
    void accept(Visitor&) final;
  };


  // Visitor
  struct Visitor
  {
    virtual void visit(Expr&)         = 0; // error
    virtual void visit(Operator& n)   = 0;
    virtual void visit(Eq&)           = 0;
    virtual void visit(StrictEq&)     = 0;
    virtual void visit(Neq&)          = 0;
    virtual void visit(StrictNeq&)    = 0;
    virtual void visit(Less&)         = 0;
    virtual void visit(Greater&)      = 0;
    virtual void visit(Leq&)          = 0;
    virtual void visit(Geq&)          = 0;
    virtual void visit(And&)          = 0;
    virtual void visit(Or&)           = 0;
    virtual void visit(Not&)          = 0;
    virtual void visit(NotNot&)       = 0;
    virtual void visit(Add&)          = 0;
    virtual void visit(Sub&)          = 0;
    virtual void visit(Mul&)          = 0;
    virtual void visit(Div&)          = 0;
    virtual void visit(Mod&)          = 0;
    virtual void visit(Min&)          = 0;
    virtual void visit(Max&)          = 0;
    virtual void visit(Map&)          = 0;
    virtual void visit(Reduce&)       = 0;
    virtual void visit(Filter&)       = 0;
    virtual void visit(All&)          = 0;
    virtual void visit(None&)         = 0;
    virtual void visit(Some&)         = 0;
    virtual void visit(Array&)        = 0;
    virtual void visit(Merge&)        = 0;
    virtual void visit(Cat&)          = 0;
    virtual void visit(Substr&)       = 0;
    virtual void visit(In&)           = 0;
    virtual void visit(Var&)          = 0;
    virtual void visit(Log&)          = 0;

    // control structure
    virtual void visit(If&)           = 0;

    // values
    virtual void visit(NullVal&)      = 0;
    virtual void visit(BoolVal&)      = 0;
    virtual void visit(IntVal&)       = 0;
    virtual void visit(UintVal&)      = 0;
    virtual void visit(DoubleVal&)    = 0;
    virtual void visit(StringVal&)    = 0;
    //~ virtual void visit(ObjectVal&)    = 0;

    virtual void visit(Error&)        = 0;
  };


  struct FwdVisitor : Visitor
  {
    void visit(Expr&)        override;
    void visit(Operator& n)  override { visit(as<Operator>(n)); }
    void visit(Eq& n)        override { visit(as<Operator>(n)); }
    void visit(StrictEq& n)  override { visit(as<Operator>(n)); }
    void visit(Neq& n)       override { visit(as<Operator>(n)); }
    void visit(StrictNeq& n) override { visit(as<Operator>(n)); }
    void visit(Less& n)      override { visit(as<Operator>(n)); }
    void visit(Greater& n)   override { visit(as<Operator>(n)); }
    void visit(Leq& n)       override { visit(as<Operator>(n)); }
    void visit(Geq& n)       override { visit(as<Operator>(n)); }
    void visit(And& n)       override { visit(as<Operator>(n)); }
    void visit(Or& n)        override { visit(as<Operator>(n)); }
    void visit(Not& n)       override { visit(as<Operator>(n)); }
    void visit(NotNot& n)    override { visit(as<Operator>(n)); }
    void visit(Add& n)       override { visit(as<Operator>(n)); }
    void visit(Sub& n)       override { visit(as<Operator>(n)); }
    void visit(Mul& n)       override { visit(as<Operator>(n)); }
    void visit(Div& n)       override { visit(as<Operator>(n)); }
    void visit(Mod& n)       override { visit(as<Operator>(n)); }
    void visit(Min& n)       override { visit(as<Operator>(n)); }
    void visit(Max& n)       override { visit(as<Operator>(n)); }
    void visit(Map& n)       override { visit(as<Operator>(n)); }
    void visit(Reduce& n)    override { visit(as<Operator>(n)); }
    void visit(Filter& n)    override { visit(as<Operator>(n)); }
    void visit(All& n)       override { visit(as<Operator>(n)); }
    void visit(None& n)      override { visit(as<Operator>(n)); }
    void visit(Some& n)      override { visit(as<Operator>(n)); }
    void visit(Array& n)     override { visit(as<Operator>(n)); }
    void visit(Merge& n)     override { visit(as<Operator>(n)); }
    void visit(Cat& n)       override { visit(as<Operator>(n)); }
    void visit(Substr& n)    override { visit(as<Operator>(n)); }
    void visit(In& n)        override { visit(as<Operator>(n)); }
    void visit(Var& n)       override { visit(as<Operator>(n)); }
    void visit(Log& n)       override { visit(as<Operator>(n)); }

    void visit(If& n)        override { visit(as<Expr>(n)); }

    void visit(NullVal& n)   override { visit(as<Expr>(n)); }
    void visit(BoolVal& n)   override { visit(as<Expr>(n)); }
    void visit(IntVal& n)    override { visit(as<Expr>(n)); }
    void visit(UintVal& n)   override { visit(as<Expr>(n)); }
    void visit(DoubleVal& n) override { visit(as<Expr>(n)); }
    void visit(StringVal& n) override { visit(as<Expr>(n)); }

    void visit(Error& n)     override { visit(as<Expr>(n)); }
  };

  /// AST traversal function that calls v's visit methods in post-fix order
  void traverseInSAttributeOrder(Expr& e, Visitor& vis);

  /// traverses the children of a node; does not traverse grandchildren
  void traverseChildren(Visitor& v, const Operator& node);
  void traverseAllChildren(Visitor& v, const Operator& node);
  void traverseChildrenReverse(Visitor& v, const Operator& node);

  namespace
  {
    CXX_NORETURN
    void unsupported()
    {
      throw std::logic_error("not yet implemented");
    }

    CXX_NORETURN
    void typeError()
    {
      throw std::runtime_error("typing error");
    }

    struct VarMap
    {
        void insert(Var& el);
        std::vector<json::string> toVector() const;
        bool hasComputedVariables() const { return hasComputed; }

      private:
        using container_type = std::map<json::string, int>;

        container_type mapping     = {};
        bool           hasComputed = false;
    };

    void VarMap::insert(Var& var)
    {
      AnyExpr&   arg = var.back();
      StringVal* str = dynamic_cast<StringVal*>(arg.get());

      if (str == nullptr)
      {
        CXX_UNLIKELY;
        hasComputed = true;
        return;
      }

      // do nothing for free variables in "lambdas"
      if (str->value() == "")
        return;

      auto [pos, success] = mapping.emplace(str->value(), mapping.size());

      var.num(pos->second);
    }

    std::vector<json::string>
    VarMap::toVector() const
    {
      std::vector<json::string> res;

      res.resize(mapping.size());

      for (const container_type::value_type& el : mapping)
        res.at(el.second) = el.first;

      return res;
    }


    /// translates all children
    /// \{
    Operator::container_type
    translateChildren(json::array& children, VarMap&);

    Operator::container_type
    translateChildren(JsonExpr& n, VarMap&);
    /// \}


    template <class ExprT>
    Expr& mkOperator(json::object& n, VarMap& m)
    {
      assert(n.size() == 1);

      ExprT& res = deref(new ExprT);

      res.set_operands(translateChildren(n.begin()->value(), m));
      return res;
    }

    Array& mkArrayOperator(json::array& children, VarMap& m)
    {
      Array& res = deref(new Array);

      res.set_operands(translateChildren(children, m));
      return res;
    }

    template <class ValueT>
    ValueT& mkValue(typename ValueT::value_type n)
    {
      return deref(new ValueT(std::move(n)));
    }

    NullVal& mkNullValue()
    {
      return deref(new NullVal);
    }

    using DispatchTable = std::map<json::string, Expr&(*)(json::object&, VarMap&)>;

    DispatchTable::const_iterator
    lookup(const DispatchTable& m, const json::object& op)
    {
      if (op.size() != 1) return m.end();

      return m.find(op.begin()->key());
    }


    AnyExpr
    translateNode_internal(JsonExpr& n, VarMap& varmap)
    {
      static const DispatchTable dt = { { "==",     &mkOperator<Eq> },
                                        { "===",    &mkOperator<StrictEq> },
                                        { "!=",     &mkOperator<Neq> },
                                        { "!==",    &mkOperator<StrictNeq> },
                                        { "if",     &mkOperator<If> },
                                        { "!",      &mkOperator<Not> },
                                        { "!!",     &mkOperator<NotNot> },
                                        { "or",     &mkOperator<Or> },
                                        { "and",    &mkOperator<And> },
                                        { ">",      &mkOperator<Greater> },
                                        { ">=",     &mkOperator<Geq> },
                                        { "<",      &mkOperator<Less> },
                                        { "<=",     &mkOperator<Leq> },
                                        { "max",    &mkOperator<Max> },
                                        { "min",    &mkOperator<Min> },
                                        { "+",      &mkOperator<Add> },
                                        { "-",      &mkOperator<Sub> },
                                        { "*",      &mkOperator<Mul> },
                                        { "/",      &mkOperator<Div> },
                                        { "%",      &mkOperator<Mod> },
                                        { "map",    &mkOperator<Map> },
                                        { "reduce", &mkOperator<Reduce> },
                                        { "filter", &mkOperator<Filter> },
                                        { "all",    &mkOperator<All> },
                                        { "none",   &mkOperator<None> },
                                        { "some",   &mkOperator<Some> },
                                        { "merge",  &mkOperator<Merge> },
                                        { "in",     &mkOperator<In> },
                                        { "cat",    &mkOperator<Cat> },
                                        { "log",    &mkOperator<Log> },
                                        { "var",    &mkOperator<Var> }
                                      };

      Expr* res = nullptr;

      switch (n.kind())
      {
        case json::kind::object:
          {
            json::object&                 obj = n.get_object();
            DispatchTable::const_iterator pos = lookup(dt, obj);

            if (pos != dt.end())
            {
              CXX_LIKELY;
              res = &pos->second(obj, varmap);

              if (pos->second == mkOperator<Var>)
                varmap.insert(dynamic_cast<Var&>(*res));
            }
            else
            {
              // does json_logic support value objects?
              unsupported();
            }

            break;
          }

        case json::kind::array:
          {
            // array is an operator that combines its subexpressions into an array
            res = &mkArrayOperator(n.get_array(), varmap);
            break;
          }

        case json::kind::string:
          {
            res = &mkValue<StringVal>(std::move(n.get_string()));
            break;
          }

        case json::kind::int64:
          {
            res = &mkValue<IntVal>(n.get_int64());
            break;
          }

        case json::kind::uint64:
          {
            res = &mkValue<UintVal>(n.get_uint64());
            break;
          }

        case json::kind::double_:
          {
            res = &mkValue<DoubleVal>(n.get_double());
            break;
          }

        case json::kind::bool_:
          {
            res = &mkValue<BoolVal>(n.get_bool());
            break;
          }

        case json::kind::null:
          {
            res = &mkNullValue();
            break;
          }

        default:
          unsupported();
      }

      return std::unique_ptr<Expr>(res);
    }


    std::tuple<AnyExpr, std::vector<json::string>, bool>
    translateNode(JsonExpr& n)
    {
      VarMap  varmap;
      AnyExpr node = translateNode_internal(n, varmap);
      bool    hasComputedVariables = varmap.hasComputedVariables();

      return std::make_tuple(std::move(node), varmap.toVector(), hasComputedVariables);
    }

    Operator::container_type
    translateChildren(json::array& children, VarMap& varmap)
    {
      Operator::container_type res;

      res.reserve(children.size());

      for (JsonExpr& elem : children)
        res.emplace_back(translateNode_internal(elem, varmap));

      return res;
    }

    Operator::container_type
    translateChildren(JsonExpr& n, VarMap& varmap)
    {
      if (json::array* arr = n.if_array())
      {
        CXX_LIKELY;
        return translateChildren(*arr, varmap);
      }

      Operator::container_type res;

      res.emplace_back(translateNode_internal(n, varmap));
      return res;
    }
  }

  // only operators have children
  void _traverseChildren(Visitor& v, Operator::const_iterator aa, Operator::const_iterator zz)
  {
    std::for_each( aa, zz,
                   [&v](const AnyExpr& e) -> void
                   {
                     e->accept(v);
                   }
                 );
  }

  void traverseChildren(Visitor& v, const Operator& node)
  {
    Operator::const_iterator aa = node.begin();

    _traverseChildren(v, aa, aa + node.num_evaluated_operands());
  }

  void traverseAllChildren(Visitor& v, const Operator& node)
  {
    _traverseChildren(v, node.begin(), node.end());
  }

  void traverseChildrenReverse(Visitor& v, const Operator& node)
  {
    Operator::const_reverse_iterator zz = node.crend();
    Operator::const_reverse_iterator aa = zz - node.num_evaluated_operands();

    std::for_each( aa, zz,
                   [&v](const AnyExpr& e) -> void
                   {
                     e->accept(v);
                   }
                 );
  }


  namespace
  {
    struct SAttributeTraversal : Visitor
    {
        explicit
        SAttributeTraversal(Visitor& client)
        : sub(client)
        {}

        void visit(Expr&)        final;
        void visit(Operator&)    final;
        void visit(Eq&)          final;
        void visit(StrictEq&)    final;
        void visit(Neq&)         final;
        void visit(StrictNeq&)   final;
        void visit(Less&)        final;
        void visit(Greater&)     final;
        void visit(Leq&)         final;
        void visit(Geq&)         final;
        void visit(And&)         final;
        void visit(Or&)          final;
        void visit(Not&)         final;
        void visit(NotNot&)      final;
        void visit(Add&)         final;
        void visit(Sub&)         final;
        void visit(Mul&)         final;
        void visit(Div&)         final;
        void visit(Mod&)         final;
        void visit(Min&)         final;
        void visit(Max&)         final;
        void visit(Map&)         final;
        void visit(Reduce&)      final;
        void visit(Filter&)      final;
        void visit(All&)         final;
        void visit(None&)        final;
        void visit(Some&)        final;
        void visit(Merge&)       final;
        void visit(Cat&)         final;
        void visit(Substr&)      final;
        void visit(In&)          final;
        void visit(Array& n)     final;
        void visit(Var&)         final;
        void visit(Log&)         final;

        void visit(If&)          final;

        void visit(NullVal& n)   final;
        void visit(BoolVal& n)   final;
        void visit(IntVal& n)    final;
        void visit(UintVal& n)   final;
        void visit(DoubleVal& n) final;
        void visit(StringVal& n) final;

        void visit(Error& n)     final;

      private:
        Visitor& sub;

        template <class OperatorNode>
        inline
        void _visit(OperatorNode& n)
        {
          traverseChildren(*this, n);
          sub.visit(n);
        }

        template <class ValueNode>
        inline
        void _value(ValueNode& n)
        {
          sub.visit(n);
        }
    };

    void SAttributeTraversal::visit(Expr&)           { typeError();    }
    void SAttributeTraversal::visit(Operator&)       { typeError();    }
    void SAttributeTraversal::visit(Eq& n)           { _visit(n); }
    void SAttributeTraversal::visit(StrictEq& n)     { _visit(n); }
    void SAttributeTraversal::visit(Neq& n)          { _visit(n); }
    void SAttributeTraversal::visit(StrictNeq& n)    { _visit(n); }
    void SAttributeTraversal::visit(Less& n)         { _visit(n); }
    void SAttributeTraversal::visit(Greater& n)      { _visit(n); }
    void SAttributeTraversal::visit(Leq& n)          { _visit(n); }
    void SAttributeTraversal::visit(Geq& n)          { _visit(n); }
    void SAttributeTraversal::visit(And& n)          { _visit(n); }
    void SAttributeTraversal::visit(Or& n)           { _visit(n); }
    void SAttributeTraversal::visit(Not& n)          { _visit(n); }
    void SAttributeTraversal::visit(NotNot& n)       { _visit(n); }
    void SAttributeTraversal::visit(Add& n)          { _visit(n); }
    void SAttributeTraversal::visit(Sub& n)          { _visit(n); }
    void SAttributeTraversal::visit(Mul& n)          { _visit(n); }
    void SAttributeTraversal::visit(Div& n)          { _visit(n); }
    void SAttributeTraversal::visit(Mod& n)          { _visit(n); }
    void SAttributeTraversal::visit(Min& n)          { _visit(n); }
    void SAttributeTraversal::visit(Max& n)          { _visit(n); }
    void SAttributeTraversal::visit(Array& n)        { _visit(n); }
    void SAttributeTraversal::visit(Map& n)          { _visit(n); }
    void SAttributeTraversal::visit(Reduce& n)       { _visit(n); }
    void SAttributeTraversal::visit(Filter& n)       { _visit(n); }
    void SAttributeTraversal::visit(All& n)          { _visit(n); }
    void SAttributeTraversal::visit(None& n)         { _visit(n); }
    void SAttributeTraversal::visit(Some& n)         { _visit(n); }
    void SAttributeTraversal::visit(Merge& n)        { _visit(n); }
    void SAttributeTraversal::visit(Cat& n)          { _visit(n); }
    void SAttributeTraversal::visit(Substr& n)       { _visit(n); }
    void SAttributeTraversal::visit(In& n)           { _visit(n); }
    void SAttributeTraversal::visit(Var& n)          { _visit(n); }
    void SAttributeTraversal::visit(Log& n)          { _visit(n); }

    void SAttributeTraversal::visit(If& n)           { _visit(n); }

    void SAttributeTraversal::visit(NullVal& n)      { _value(n); }
    void SAttributeTraversal::visit(BoolVal& n)      { _value(n); }
    void SAttributeTraversal::visit(IntVal& n)       { _value(n); }
    void SAttributeTraversal::visit(UintVal& n)      { _value(n); }
    void SAttributeTraversal::visit(DoubleVal& n)    { _value(n); }
    void SAttributeTraversal::visit(StringVal& n)    { _value(n); }

    void SAttributeTraversal::visit(Error& n)        { sub.visit(n); }
  }

  using ValueExpr = std::unique_ptr<Expr>; // could be value if we have a type for that

  std::ostream& operator<<(std::ostream& os, ValueExpr& n);

  ValueExpr toValueExpr(std::nullptr_t)    { return ValueExpr(new NullVal); }
  ValueExpr toValueExpr(bool val)          { return ValueExpr(new BoolVal(val)); }
  ValueExpr toValueExpr(std::int64_t val)  { return ValueExpr(new IntVal(val)); }
  ValueExpr toValueExpr(std::uint64_t val) { return ValueExpr(new UintVal(val)); }
  ValueExpr toValueExpr(double val)        { return ValueExpr(new DoubleVal(val)); }
  ValueExpr toValueExpr(json::string val)  { return ValueExpr(new StringVal(std::move(val))); }

  //
  // coercion functions

  std::int64_t toInt(const json::string& str)
  {
    return std::stoi(std::string{str.c_str()});
  }

  std::int64_t toInt(double d)
  {
    return d;
  }

  std::int64_t toInt(bool b)
  {
    return b;
  }

  std::int64_t toInt(std::uint64_t v)
  {
    return v;
  }

  std::uint64_t toUint(const json::string& str)
  {
    return std::stoull(std::string{str.c_str()});
  }

  std::uint64_t toUint(double d)
  {
    return d;
  }

  std::uint64_t toUint(std::int64_t v)
  {
    return v;
  }

  std::uint64_t toUint(bool b)
  {
    return b;
  }

  double toDouble(const json::string& str)
  {
    return std::stod(std::string{str.c_str()});
  }

  double toDouble(std::int64_t val)
  {
    return val;
  }

  double toDouble(std::uint64_t val)
  {
    return val;
  }


  template <class Val>
  json::string toString(Val v)
  {
    return json::string{std::to_string(v)};
  }

  json::string toString(bool b)
  {
    return json::string{b ? "true" : "false"};
  }

  json::string toString(std::nullptr_t)
  {
    return json::string{"null"};
  }

  /// conversion to boolean
  /// \{

  bool toBool(std::int64_t v)        { return v; }
  bool toBool(std::uint64_t v)       { return v; }
  bool toBool(double v)              { return v; }
  bool toBool(const json::string& v) { return v.size() != 0; }
  bool toBool(Array& v)              { return v.num_evaluated_operands(); }

  bool toBool(Expr& e)
  {
    struct BoolConverter : FwdVisitor
    {
      void visit(Expr&)        final { typeError(); }

      void visit(NullVal&)     final { res = false; }
      void visit(BoolVal& n)   final { res = n.value(); }
      void visit(IntVal& n)    final { res = toBool(n.value()); }
      void visit(UintVal& n)   final { res = toBool(n.value()); }
      void visit(DoubleVal& n) final { res = toBool(n.value()); }
      void visit(StringVal& n) final { res = toBool(n.value()); }
      void visit(Array& n)     final { res = toBool(n); }

      bool res;
    };

    BoolConverter conv;

    e.accept(conv);
    return conv.res;
  }

  bool toBool(ValueExpr& el)
  {
    return toBool(*el);
  }

  /// \}


  struct LogicalOperatorBase
  {
    enum
    {
      definedForString  = true,
      definedForDouble  = true,
      definedForInteger = true,
      definedForBool    = false,
      definedForNull    = false
    };

    using result_type   = bool;
  };

  struct NoCoercion {};

  /// \brief a strict binary operator operates on operands of the same
  ///        type. The operation on two different types returns false.
  ///        NO type coercion is performed.
  struct StrictLogicalBinaryOperator : LogicalOperatorBase
  {
    template <class LhsT, class RhsT>
    std::tuple<LhsT, RhsT>
    coerce(LhsT* lv, RhsT* rv)
    {
      return std::make_tuple(std::move(*lv), std::move(*rv));
    }
  };

  struct NumericBinaryOperatorBase
  {
    std::tuple<double, double>
    coerce(double* lv, double* rv)
    {
      return std::make_tuple(*lv, *rv);
    }

    std::tuple<double, double>
    coerce(double* lv, std::int64_t* rv)
    {
      return std::make_tuple(*lv, toDouble(*rv));
    }

    std::tuple<double, double>
    coerce(double* lv, std::uint64_t* rv)
    {
      return std::make_tuple(*lv, toDouble(*rv));
    }

    std::tuple<double, double>
    coerce(std::int64_t* lv, double* rv)
    {
      return std::make_tuple(toDouble(*lv), *rv);
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, std::int64_t* rv)
    {
      return std::make_tuple(*lv, *rv);
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, std::uint64_t* rv)
    {
      return std::make_tuple(*lv, toInt(*rv));
    }

    std::tuple<double, double>
    coerce(std::uint64_t* lv, double* rv)
    {
      return std::make_tuple(toDouble(*lv), *rv);
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::uint64_t* lv, std::int64_t* rv)
    {
      return std::make_tuple(toInt(*lv), *rv);
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::uint64_t* lv, std::uint64_t* rv)
    {
      return std::make_tuple(*lv, *rv);
    }
  };


  /// \brief a logical binary operator compares two values. If the
  ///        values have a different type, type coercion is performed
  ///        on one of the operands.
  struct LogicalBinaryOperator : NumericBinaryOperatorBase, LogicalOperatorBase
  {
    using NumericBinaryOperatorBase::coerce;

    std::tuple<double, double>
    coerce(double* lv, json::string* rv)
    {
      return std::make_tuple(*lv, toDouble(*rv));
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, json::string* rv)
    {
      return std::make_tuple(*lv, toInt(*rv));
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::uint64_t* lv, json::string* rv)
    {
      return std::make_tuple(*lv, toUint(*rv));
    }

    std::tuple<double, double>
    coerce(json::string* lv, double* rv)
    {
      return std::make_tuple(toDouble(*lv), *rv);
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(json::string* lv, std::int64_t* rv)
    {
      return std::make_tuple(toInt(*lv), *rv);
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(json::string* lv, std::uint64_t* rv)
    {
      return std::make_tuple(toInt(*lv), *rv);
    }

    std::tuple<json::string, json::string>
    coerce(json::string* lv, json::string* rv)
    {
      return std::make_tuple(std::move(*lv), std::move(*rv));
    }
  };
  // @}

  // Arith
  struct ArithmeticOperator : NumericBinaryOperatorBase
  {
    enum
    {
      definedForString  = false,
      definedForDouble  = true,
      definedForInteger = true,
      definedForBool    = false,
      definedForNull    = true
    };

    using result_type = ValueExpr;

    using NumericBinaryOperatorBase::coerce;

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(double*, std::nullptr_t)
    {
      return std::make_tuple(nullptr, nullptr);
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::int64_t*, std::nullptr_t)
    {
      return std::make_tuple(nullptr, nullptr);
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::uint64_t*, std::nullptr_t)
    {
      return std::make_tuple(nullptr, nullptr);
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, double*)
    {
      return std::make_tuple(nullptr, nullptr);
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::int64_t*)
    {
      return std::make_tuple(nullptr, nullptr);
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::uint64_t*)
    {
      return std::make_tuple(nullptr, nullptr);
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::nullptr_t)
    {
      return std::make_tuple(nullptr, nullptr);
    }
  };

  struct IntegerArithmeticOperator : ArithmeticOperator
  {
    enum
    {
      definedForString  = false,
      definedForDouble  = false,
      definedForInteger = true,
      definedForBool    = false,
      definedForNull    = false
    };

    using ArithmeticOperator::coerce;
  };

  struct StringOperator
  {
    enum
    {
      definedForString  = true,
      definedForDouble  = false,
      definedForInteger = false,
      definedForBool    = false,
      definedForNull    = false
    };

    using result_type = ValueExpr;

    std::tuple<json::string, json::string>
    coerce(json::string* lv, json::string* rv)
    {
      return std::make_tuple(std::move(*lv), std::move(*rv));
    }
  };


  AnyExpr convert(AnyExpr val, ...)
  {
    return val;
  }


  AnyExpr convert(AnyExpr val, const ArithmeticOperator&)
  {
    struct ArithmeticConverter : FwdVisitor
    {
        explicit
        ArithmeticConverter(AnyExpr val)
        : res(std::move(val))
        {}

        void visit(Expr&)         final { typeError(); }

        // defined for the following types
        void visit(IntVal&)       final {}
        void visit(UintVal&)      final {}
        void visit(DoubleVal&)    final {}
        void visit(NullVal&)      final {}

        // need to convert values
        void visit(StringVal& el) final
        {
          double  dd = toDouble(el.value());
          int64_t ii = toInt(el.value());
          // uint?

          res = (dd != ii) ? toValueExpr(dd) : toValueExpr(ii);
        }

        void visit(BoolVal&) final
        {
          // \todo correct?
          res = toValueExpr(nullptr);
        }

        AnyExpr result() && { return std::move(res); }
      private:
        AnyExpr res;
    };

    Expr*               node = val.get();
    ArithmeticConverter conv{std::move(val)};

    node->accept(conv);
    return std::move(conv).result();
  }

  AnyExpr convert(AnyExpr val, const IntegerArithmeticOperator&)
  {
    struct IntegerArithmeticConverter : FwdVisitor
    {
        explicit
        IntegerArithmeticConverter(AnyExpr val)
        : res(std::move(val))
        {}

        void visit(Expr&)         final { typeError(); }

        // defined for the following types
        void visit(IntVal&)       final {}
        void visit(UintVal&)      final {}

        // need to convert values
        void visit(StringVal& el) final
        {
          res = toValueExpr(toInt(el.value()));
        }

        void visit(BoolVal& el) final
        {
          res = toValueExpr(toInt(el.value()));
        }

        void visit(DoubleVal& el) final
        {
          res = toValueExpr(toInt(el.value()));
        }

        void visit(NullVal&)      final
        {
          res = toValueExpr(std::int64_t(0));
        }

        AnyExpr result() && { return std::move(res); }

      private:
        AnyExpr res;
    };

    Expr*                      node = val.get();
    IntegerArithmeticConverter conv{std::move(val)};

    node->accept(conv);
    return std::move(conv).result();
  }

  AnyExpr convert(AnyExpr val, const StringOperator&)
  {
    struct StringConverter : FwdVisitor
    {
        explicit
        StringConverter(AnyExpr val)
        : res(std::move(val))
        {}

        void visit(Expr&)         final { typeError(); }

        // defined for the following types
        void visit(StringVal&)    final {}


        // need to convert values
        void visit(BoolVal& el) final
        {
          res = toValueExpr(toString(el.value()));
        }

        void visit(IntVal& el)    final
        {
          res = toValueExpr(toString(el.value()));
        }

        void visit(UintVal& el)   final
        {
          res = toValueExpr(toString(el.value()));
        }

        void visit(DoubleVal& el) final
        {
          res = toValueExpr(toString(el.value()));
        }

        void visit(NullVal& el)      final
        {
          res = toValueExpr(toString(el.value()));
        }

        AnyExpr result() && { return std::move(res); }

      private:
        AnyExpr res;
    };

    Expr*           node = val.get();
    StringConverter conv{std::move(val)};

    node->accept(conv);
    return std::move(conv).result();
  }

  template <class BinaryOperator, class LhsValue>
  struct BinaryOperatorVisitor_ : FwdVisitor
  {
      using result_type = typename BinaryOperator::result_type;

      BinaryOperatorVisitor_(LhsValue lval, BinaryOperator oper)
      : lv(lval), op(oper), res()
      {}

      template <class RhsValue>
      void calc(RhsValue rv)
      {
        auto [ll, rr] = op.coerce(lv, rv);

        res = op(ll, rr);
      }

      void visit(Expr&) final { typeError(); }

      void visit(StringVal& n) final
      {
        if constexpr (BinaryOperator::definedForString)
          return calc(&n.value());

        typeError();
      }

      void visit(NullVal&) final
      {
        if constexpr (BinaryOperator::definedForNull)
          return calc(nullptr);

        typeError();
      }

      void visit(BoolVal& n) final
      {
        if constexpr (BinaryOperator::definedForBool)
          return calc(&n.value());

        typeError();
      }

      void visit(IntVal& n) final
      {
        if constexpr (BinaryOperator::definedForInteger)
          return calc(&n.value());

        typeError();
      }

      void visit(UintVal& n) final
      {
        if constexpr (BinaryOperator::definedForInteger)
          return calc(&n.value());

        typeError();
      }

      void visit(DoubleVal& n) final
      {
        if constexpr (BinaryOperator::definedForDouble)
          return calc(&n.value());

        typeError();
      }

      void visit(Array&) final
      {
        unsupported(); // for now
      }

      result_type result() && { return std::move(res); }

    private:
      LhsValue       lv;
      BinaryOperator op;
      result_type    res;
  };

  template <class BinaryOperator>
  struct BinaryOperatorVisitor : FwdVisitor
  {
      using result_type = typename BinaryOperator::result_type;

      BinaryOperatorVisitor(BinaryOperator oper, ValueExpr& rhsarg)
      : op(oper), rhs(rhsarg), res()
      {}

      template <class LhsValue>
      void calc(LhsValue lv)
      {
        using RhsVisitor = BinaryOperatorVisitor_<BinaryOperator, LhsValue>;

        RhsVisitor vis{lv, op};

        rhs->accept(vis);
        res = std::move(vis).result();
      }

      void visit(StringVal& n) final
      {
        if constexpr (BinaryOperator::definedForString)
          return calc(&n.value());

        typeError();
      }

      void visit(NullVal&) final
      {
        if constexpr (BinaryOperator::definedForNull)
          return calc(nullptr);

        typeError();
      }

      void visit(BoolVal& n) final
      {
        if constexpr (BinaryOperator::definedForBool)
          return calc(&n.value());

        typeError();
      }

      void visit(IntVal& n) final
      {
        if constexpr (BinaryOperator::definedForInteger)
          return calc(&n.value());

        typeError();
      }

      void visit(UintVal& n) final
      {
        if constexpr (BinaryOperator::definedForInteger)
          return calc(&n.value());

        typeError();
      }

      void visit(DoubleVal& n) final
      {
        if constexpr (BinaryOperator::definedForDouble)
          return calc(&n.value());

        typeError();
      }

      void visit(Array&) final
      {
        typeError(); // for now
      }

      result_type result() && { return std::move(res); }

    private:
      BinaryOperator op;
      ValueExpr&     rhs;
      result_type    res;
  };

  template <class BinaryOperator>
  typename BinaryOperator::result_type
  compute(ValueExpr& lhs, ValueExpr& rhs, BinaryOperator op)
  {
    using LhsVisitor = BinaryOperatorVisitor<BinaryOperator>;

    LhsVisitor vis{op, rhs};

    assert(lhs.get());
    lhs->accept(vis);
    return std::move(vis).result();
  }



  template <class>
  struct Calc {};


  template <>
  struct Calc<Eq> : LogicalBinaryOperator
  {
    using LogicalBinaryOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs == rhs;
    }
  };

  template <>
  struct Calc<Neq> : LogicalBinaryOperator
  {
    using LogicalBinaryOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs != rhs;
    }
  };

  template <>
  struct Calc<StrictEq> : StrictLogicalBinaryOperator
  {
    using StrictLogicalBinaryOperator::result_type;

    result_type operator()(...) const { return false; } // type mismatch

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs == rhs;
    }
  };

  template <>
  struct Calc<StrictNeq> : StrictLogicalBinaryOperator
  {
    using StrictLogicalBinaryOperator::result_type;

    result_type operator()(...) const { return false; } // type mismatch

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs == rhs;
    }
  };

  template <>
  struct Calc<Less> : LogicalBinaryOperator
  {
    using LogicalBinaryOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs < rhs;
    }
  };

  template <>
  struct Calc<Greater> : LogicalBinaryOperator
  {
    using LogicalBinaryOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return rhs < lhs;
    }
  };

  template <>
  struct Calc<Leq> : LogicalBinaryOperator
  {
    using LogicalBinaryOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs <= rhs;
    }
  };

  template <>
  struct Calc<Geq> : LogicalBinaryOperator
  {
    using LogicalBinaryOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return rhs <= lhs;
    }
  };

  template <>
  struct Calc<Add> : ArithmeticOperator
  {
    using ArithmeticOperator::result_type;

    result_type
    operator()(std::nullptr_t, std::nullptr_t) const { return toValueExpr(nullptr); }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return toValueExpr(lhs + rhs);
    }
  };

  template <>
  struct Calc<Sub> : ArithmeticOperator
  {
    using ArithmeticOperator::result_type;

    result_type
    operator()(std::nullptr_t, std::nullptr_t) const { return toValueExpr(nullptr); }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return toValueExpr(lhs - rhs);
    }
  };

  template <>
  struct Calc<Mul> : ArithmeticOperator
  {
    using ArithmeticOperator::result_type;

    result_type
    operator()(std::nullptr_t, std::nullptr_t) const { return toValueExpr(nullptr); }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return toValueExpr(lhs * rhs);
    }
  };

  template <>
  struct Calc<Div> : ArithmeticOperator
  {
    using ArithmeticOperator::result_type;

    result_type
    operator()(std::nullptr_t, std::nullptr_t) const { return toValueExpr(nullptr); }

    result_type
    operator()(double lhs, double rhs) const
    {
      double res = lhs / rhs;

      // if (isInteger(res)) return toInt(res);
      return toValueExpr(res);
    }

    template <class Int_t>
    result_type
    operator()(Int_t lhs, Int_t rhs) const
    {
      if (lhs % rhs) return (*this)(double(lhs), double(rhs));

      return toValueExpr(lhs / rhs);
    }
  };

  template <>
  struct Calc<Mod> : IntegerArithmeticOperator
  {
    using IntegerArithmeticOperator::result_type;

    std::nullptr_t
    operator()(std::nullptr_t, std::nullptr_t) const { return nullptr; }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      if (rhs == 0) return toValueExpr(nullptr);

      return toValueExpr(lhs % rhs);
    }
  };

  template <>
  struct Calc<Min> : ArithmeticOperator
  {
    using ArithmeticOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return toValueExpr(std::min(lhs, rhs));
    }
  };

  template <>
  struct Calc<Max> : ArithmeticOperator
  {
    using ArithmeticOperator::result_type;

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return toValueExpr(std::max(lhs, rhs));
    }
  };

  template <>
  struct Calc<Not>
  {
    using result_type = bool;

    result_type
    operator()(Expr& val) const
    {
      return !toBool(val);
    }
  };

  template <>
  struct Calc<NotNot>
  {
    using result_type = bool;

    result_type
    operator()(Expr& val) const
    {
      return toBool(val);
    }
  };

  template <>
  struct Calc<Cat> : StringOperator
  {
    using StringOperator::result_type;

    result_type
    operator()(const json::string& lhs, const json::string& rhs) const
    {
      json::string tmp;

      tmp.reserve(lhs.size()+rhs.size());

      tmp.append(lhs.begin(), lhs.end());
      tmp.append(rhs.begin(), rhs.end());

      return toValueExpr(std::move(tmp));
    }
  };

  struct Calculator : FwdVisitor
  {
      using VarAccess = std::function<ValueExpr(const json::string&, int)>;

      Calculator(const VarAccess& varAccess, std::ostream& out)
      : vars(varAccess), logger(out), calcres(nullptr)
      {}

      void visit(Eq&)          final;
      void visit(StrictEq&)    final;
      void visit(Neq&)         final;
      void visit(StrictNeq&)   final;
      void visit(Less&)        final;
      void visit(Greater&)     final;
      void visit(Leq&)         final;
      void visit(Geq&)         final;
      void visit(And&)         final;
      void visit(Or&)          final;
      void visit(Not&)         final;
      void visit(NotNot&)      final;
      void visit(Add&)         final;
      void visit(Sub&)         final;
      void visit(Mul&)         final;
      void visit(Div&)         final;
      void visit(Mod&)         final;
      void visit(Min&)         final;
      void visit(Max&)         final;
      void visit(Array&)       final;
      void visit(Map&)         final;
      void visit(Reduce&)      final;
      void visit(Filter&)      final;
      void visit(All&)         final;
      void visit(None&)        final;
      void visit(Some&)        final;
      void visit(Merge&)       final;
      void visit(Cat&)         final;
      void visit(Substr&)      final;
      void visit(In&)          final;
      void visit(Var&)         final;
      void visit(Log&)         final;

      void visit(NullVal& n)   final;
      void visit(BoolVal& n)   final;
      void visit(IntVal& n)    final;
      void visit(UintVal& n)   final;
      void visit(DoubleVal& n) final;
      void visit(StringVal& n) final;

      void visit(Error& n)     final;

      ValueExpr eval(Expr& n);

    private:
      const VarAccess& vars;
      std::ostream&    logger;
      ValueExpr        calcres;

      Calculator(const Calculator&)            = delete;
      Calculator(Calculator&&)                 = delete;
      Calculator& operator=(const Calculator&) = delete;
      Calculator& operator=(Calculator&&)      = delete;

      //
      // opers

      /// implements relop : [1, 2, 3, whatever]
      template <class BinaryPredicate>
      void evalPairShortCircuit(Operator& n, BinaryPredicate calc)
      {
        const int      num = n.num_evaluated_operands();
        assert(num >= 2);

        bool           res = true;
        int            idx = -1;
        ValueExpr      rhs = eval(n.operand(++idx));
        assert(rhs.get());

        while (res && (idx != (num-1)))
        {
          ValueExpr    lhs = std::move(rhs);
          assert(lhs.get());

          rhs = eval(n.operand(++idx));
          assert(rhs.get());

          res = compute(lhs, rhs, calc);
        }

        calcres = toValueExpr(res);
      }

      template <class BinaryOperator>
      void reduce(Operator& n, BinaryOperator op)
      {
        const int      num = n.num_evaluated_operands();
        assert(num >= 1);

        int            idx = -1;
        ValueExpr      res = eval(n.operand(++idx));

        res = convert(std::move(res), op);

        while (idx != (num-1))
        {
          ValueExpr rhs = eval(n.operand(++idx));

          rhs = convert(std::move(rhs), op);
          res = compute(res, rhs, op);
        }

        calcres = std::move(res);
      }

      template <class BinaryOperator>
      void binary(Operator& n, BinaryOperator calc)
      {
        const int num = n.num_evaluated_operands();
        assert(num == 1 || num == 2);

        int       idx = -1;
        ValueExpr lhs;

        if (num == 2)
        {
          CXX_LIKELY;
          lhs = eval(n.operand(++idx));
        }
        else
        {
          lhs = toValueExpr(std::int64_t(0));
        }

        ValueExpr rhs = eval(n.operand(++idx));

        calcres = compute(lhs, rhs, calc);
      }

      template <class UnaryOperator>
      void unary(Operator& n, UnaryOperator calc)
      {
        const int  num = n.num_evaluated_operands();
        assert(num == 1);

        const bool res = calc(*eval(n.operand(0)));

        calcres = toValueExpr(res);
      }

      void evalShortCircuit(Operator& n, bool val)
      {
        const int      num = n.num_evaluated_operands();
        assert(num >= 1);

        int            idx   = -1;
        ValueExpr      oper  = eval(n.operand(++idx));
        bool           found = (idx == num-1) || (toBool(*oper) == val);

        // loop until *aa == val or when *aa is the last valid element
        while (!found)
        {
          oper = eval(n.operand(++idx));

          found = (idx == (num-1)) || (toBool(*oper) == val);
        }

        calcres = std::move(oper);
      }

      template <class ValueNode>
      void _value(const ValueNode& val)
      {
        calcres = toValueExpr(val.value());
      }
  };

  ValueExpr Calculator::eval(Expr& n)
  {
    ValueExpr res;

    n.accept(*this);
    res.swap(calcres);

    return res;
  }

  void Calculator::visit(Eq& n)
  {
    evalPairShortCircuit(n, Calc<Eq>{});
  }

  void Calculator::visit(StrictEq& n)
  {
    evalPairShortCircuit(n, Calc<StrictEq>{});
  }

  void Calculator::visit(Neq& n)
  {
    evalPairShortCircuit(n, Calc<Neq>{});
  }

  void Calculator::visit(StrictNeq& n)
  {
    evalPairShortCircuit(n, Calc<StrictNeq>{});
  }

  void Calculator::visit(Less& n)
  {
    evalPairShortCircuit(n, Calc<Less>{});
  }

  void Calculator::visit(Greater& n)
  {
    evalPairShortCircuit(n, Calc<Greater>{});
  }

  void Calculator::visit(Leq& n)
  {
    evalPairShortCircuit(n, Calc<Leq>{});
  }

  void Calculator::visit(Geq& n)
  {
    evalPairShortCircuit(n, Calc<Geq>{});
  }

  void Calculator::visit(And& n)
  {
    evalShortCircuit(n, false);
  }

  void Calculator::visit(Or& n)
  {
    evalShortCircuit(n, true);
  }

  void Calculator::visit(Not& n)
  {
    unary(n, Calc<Not>{});
  }

  void Calculator::visit(NotNot& n)
  {
    unary(n, Calc<NotNot>{});
  }

  void Calculator::visit(Add& n)
  {
    reduce(n, Calc<Add>{});
  }

  void Calculator::visit(Sub& n)
  {
    binary(n, Calc<Sub>{});
  }

  void Calculator::visit(Mul& n)
  {
    reduce(n, Calc<Mul>{});
  }

  void Calculator::visit(Div& n)
  {
    binary(n, Calc<Div>{});
  }

  void Calculator::visit(Mod& n)
  {
    binary(n, Calc<Mod>{});
  }

  void Calculator::visit(Min& n)
  {
    reduce(n, Calc<Min>{});
  }

  void Calculator::visit(Max& n)
  {
    reduce(n, Calc<Max>{});
  }

  void Calculator::visit(Cat& n)
  {
    reduce(n, Calc<Cat>{});
  }

  void Calculator::visit(Substr&)         { unsupported(); }
  void Calculator::visit(Array&)          { unsupported(); }
  void Calculator::visit(Map&)            { unsupported(); }
  void Calculator::visit(Reduce&)         { unsupported(); }
  void Calculator::visit(Filter&)         { unsupported(); }
  void Calculator::visit(All&)            { unsupported(); }
  void Calculator::visit(None&)           { unsupported(); }
  void Calculator::visit(Some&)           { unsupported(); }
  void Calculator::visit(Merge&)          { unsupported(); }
  void Calculator::visit(In&)             { unsupported(); }

  void Calculator::visit(Error&)          { unsupported(); }

  void Calculator::visit(Var& n)
  {
    assert(n.num_evaluated_operands() == 1);

    AnyExpr elm = convert(eval(n.operand(0)), StringOperator{});

    if (StringVal* str = dynamic_cast<StringVal*>(elm.get()))
    {
      CXX_LIKELY;
      calcres = vars(str->value(), n.num());
      return;
    }

    typeError();
  }


  void Calculator::visit(Log&)
  {
    const int  num = n.num_evaluated_operands();
    assert(num == 1);

    calcres = calc(*eval(n.operand(0)));

    std::cerr << calcres << std::endl;
  }

  void Calculator::visit(NullVal& n)   { _value(n); }
  void Calculator::visit(BoolVal& n)   { _value(n); }
  void Calculator::visit(IntVal& n)    { _value(n); }
  void Calculator::visit(UintVal& n)   { _value(n); }
  void Calculator::visit(DoubleVal& n) { _value(n); }
  void Calculator::visit(StringVal& n) { _value(n); }

  void traverseInSAttributeOrder(Expr& e, Visitor& vis)
  {
    SAttributeTraversal trav{vis};

    e.accept(trav);
  }

  ValueExpr calculate(ValueExpr& exp, const Calculator::VarAccess& vars)
  {
    Calculator calc{vars, std::cerr};

    assert(exp.get());
    return calc.eval(*exp);
  }

  ValueExpr calculate(ValueExpr& exp)
  {
    return calculate(exp, [](const json::string&, int) -> ValueExpr { unsupported(); });
  }

  std::ostream& operator<<(std::ostream& os, ValueExpr& n)
  {
    struct ValuePrinter : FwdVisitor
    {
      explicit
      ValuePrinter(std::ostream& stream)
      : os(stream)
      {}

      void visit(NullVal&)     final { os << toString(nullptr); }
      void visit(BoolVal& n)   final { os << toString(n.value()); }
      void visit(IntVal& n)    final { os << n.value(); }
      void visit(UintVal& n)   final { os << n.value(); }
      void visit(DoubleVal& n) final { os << n.value(); }
      void visit(StringVal& n) final { os << n.value(); }
      void visit(Array&)       final { unsupported(); }

      std::ostream& os;
    };

    ValuePrinter prn{os};

    n->accept(prn);
    return os;
  }

  // accept implementations
  void Eq::accept(Visitor& v)        { v.visit(*this); }
  void StrictEq::accept(Visitor& v)  { v.visit(*this); }
  void Neq::accept(Visitor& v)       { v.visit(*this); }
  void StrictNeq::accept(Visitor& v) { v.visit(*this); }
  void Less::accept(Visitor& v)      { v.visit(*this); }
  void Greater::accept(Visitor& v)   { v.visit(*this); }
  void Leq::accept(Visitor& v)       { v.visit(*this); }
  void Geq::accept(Visitor& v)       { v.visit(*this); }
  void And::accept(Visitor& v)       { v.visit(*this); }
  void Or::accept(Visitor& v)        { v.visit(*this); }
  void Not::accept(Visitor& v)       { v.visit(*this); }
  void NotNot::accept(Visitor& v)    { v.visit(*this); }
  void Add::accept(Visitor& v)       { v.visit(*this); }
  void Sub::accept(Visitor& v)       { v.visit(*this); }
  void Mul::accept(Visitor& v)       { v.visit(*this); }
  void Div::accept(Visitor& v)       { v.visit(*this); }
  void Mod::accept(Visitor& v)       { v.visit(*this); }
  void Min::accept(Visitor& v)       { v.visit(*this); }
  void Max::accept(Visitor& v)       { v.visit(*this); }
  void Map::accept(Visitor& v)       { v.visit(*this); }
  void Reduce::accept(Visitor& v)    { v.visit(*this); }
  void Filter::accept(Visitor& v)    { v.visit(*this); }
  void All::accept(Visitor& v)       { v.visit(*this); }
  void None::accept(Visitor& v)      { v.visit(*this); }
  void Some::accept(Visitor& v)      { v.visit(*this); }
  void Array::accept(Visitor& v)     { v.visit(*this); }
  void Merge::accept(Visitor& v)     { v.visit(*this); }
  void Cat::accept(Visitor& v)       { v.visit(*this); }
  void Substr::accept(Visitor& v)    { v.visit(*this); }
  void In::accept(Visitor& v)        { v.visit(*this); }
  void Var::accept(Visitor& v)       { v.visit(*this); }
  void Log::accept(Visitor& v)       { v.visit(*this); }
  void If::accept(Visitor& v)        { v.visit(*this); }

  void NullVal::accept(Visitor& v)   { v.visit(*this); }
  void BoolVal::accept(Visitor& v)   { v.visit(*this); }
  void IntVal::accept(Visitor& v)    { v.visit(*this); }
  void UintVal::accept(Visitor& v)   { v.visit(*this); }
  void DoubleVal::accept(Visitor& v) { v.visit(*this); }
  void StringVal::accept(Visitor& v) { v.visit(*this); }

  void Error::accept(Visitor& v)     { v.visit(*this); }

  void FwdVisitor::visit(Expr&) { /* error ? */ }

  // num_evaluated_operands implementations
  int Operator::num_evaluated_operands() const
  {
    return size();
  }

  template <int MaxArity>
  int OperatorN<MaxArity>::num_evaluated_operands() const
  {
    return std::min(MaxArity, Operator::num_evaluated_operands());
  }
}

