
#pragma once

#include <exception>
#include <algorithm>
#include <string>
#include <limits>
#include <memory>
#include <regex>
#include <unordered_map>

#include <boost/json.hpp>

#include <experimental/cxx-compat.hpp>

namespace json_logic
{
  constexpr bool DEBUG_OUTPUT    = false;

  namespace json = boost::json;

  using JsonExpr = json::value;

  struct Expr;

  template <class T>
  T& up_cast(T& n) { return n; }

  template <class T>
  T& down_cast(Expr& e);

  struct cast_error : std::runtime_error
  {
    using base = std::runtime_error;
    using base::base;
  };

  struct type_error : std::runtime_error
  {
    using base = std::runtime_error;
    using base::base;
  };

  template <class Error = std::runtime_error, class T>
  T& deref(T* p, const char* msg = "assertion failed")
  {
    if (p == nullptr)
    {
      CXX_UNLIKELY;
      throw Error{msg};
    }

    return *p;
  }

  template <class Error = std::runtime_error, class T>
  T& deref(std::unique_ptr<T>& p, const char* msg = "assertion failed")
  {
    if (p.get() == nullptr)
    {
      CXX_UNLIKELY;
      throw Error{msg};
    }

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

  using AnyExpr   = std::unique_ptr<Expr>;
  using ValueExpr = std::unique_ptr<Expr>; // \todo consider std::unique_ptr<Value>

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
    using container_type::size;
    using container_type::at;

    // convenience function so that the constructor does not need to be implemented
    // in every derived class.
    void set_operands(container_type&& opers) { this->swap(opers); }

    container_type& operands() { return *this; }
    container_type&& move_operands() && { return std::move(*this); }

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

  struct Value : Expr
  {
    virtual JsonExpr toJson() const = 0;
  };

  template <class T>
  struct ValueT : Value
  {
      using value_type = T;

      explicit
      ValueT(T t)
      : val(std::move(t))
      {}

      T&       value()       { return val; }
      const T& value() const { return val; }

      JsonExpr toJson() const final;

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

  // arrays serve a dual purpose
  //   they can be considered collections, but also an aggregate value
  // The class is final and it supports move ctor/assignment, so the data
  //   can move efficiently.
  struct Array final : Operator  // array is modeled as operator
  {
      void accept(Visitor&) final;

      // define
      Array() = default;
      Array(Array&&);
      Array& operator=(Array&&);
  };

  Array::Array(Array&& other)
  : Operator()
  {
    set_operands(std::move(other).move_operands());
  }

  Array&
  Array::operator=(Array&& other)
  {
    set_operands(std::move(other).move_operands());
    return *this;
  }

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
  struct Var       : Operator
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
  struct NullVal   : Value
  {
      void accept(Visitor&) final;

      std::nullptr_t value() const { return nullptr; }

      JsonExpr       toJson() const final;
  };

  struct BoolVal   : ValueT<bool>
  {
      using base = ValueT<bool>;
      using base::base;

      void accept(Visitor&) final;
  };

  struct IntVal    : ValueT<std::int64_t>
  {
      using base = ValueT<std::int64_t>;
      using base::base;

      void accept(Visitor&) final;
  };

  struct UintVal   : ValueT<std::uint64_t>
  {
      using base = ValueT<std::uint64_t>;
      using base::base;

      void accept(Visitor&) final;
  };

  struct DoubleVal : ValueT<double>
  {
      using base = ValueT<double>;
      using base::base;

      void accept(Visitor&) final;
  };

  struct StringVal : ValueT<json::string>
  {
      using base = ValueT<json::string>;
      using base::base;

      void accept(Visitor&) final;
  };

  struct ObjectVal : Expr, private std::map<json::string, AnyExpr>
  {
      using base = std::map<json::string, AnyExpr>;
      using base::base;

      using base::iterator;
      using base::const_iterator;
      using base::find;
      using base::end;
      using base::insert;

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

  //
  // jsonlogic extensions
 //

  struct RegexMatch : OperatorN<2>
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
    virtual void visit(Value&)        = 0;
    virtual void visit(NullVal&)      = 0;
    virtual void visit(BoolVal&)      = 0;
    virtual void visit(IntVal&)       = 0;
    virtual void visit(UintVal&)      = 0;
    virtual void visit(DoubleVal&)    = 0;
    virtual void visit(StringVal&)    = 0;
    //~ virtual void visit(ObjectVal&)    = 0;

    virtual void visit(Error&)        = 0;

    // extensions
    virtual void visit(RegexMatch&)   = 0;
  };

  // accept implementations
  void Eq::accept(Visitor& v)         { v.visit(*this); }
  void StrictEq::accept(Visitor& v)   { v.visit(*this); }
  void Neq::accept(Visitor& v)        { v.visit(*this); }
  void StrictNeq::accept(Visitor& v)  { v.visit(*this); }
  void Less::accept(Visitor& v)       { v.visit(*this); }
  void Greater::accept(Visitor& v)    { v.visit(*this); }
  void Leq::accept(Visitor& v)        { v.visit(*this); }
  void Geq::accept(Visitor& v)        { v.visit(*this); }
  void And::accept(Visitor& v)        { v.visit(*this); }
  void Or::accept(Visitor& v)         { v.visit(*this); }
  void Not::accept(Visitor& v)        { v.visit(*this); }
  void NotNot::accept(Visitor& v)     { v.visit(*this); }
  void Add::accept(Visitor& v)        { v.visit(*this); }
  void Sub::accept(Visitor& v)        { v.visit(*this); }
  void Mul::accept(Visitor& v)        { v.visit(*this); }
  void Div::accept(Visitor& v)        { v.visit(*this); }
  void Mod::accept(Visitor& v)        { v.visit(*this); }
  void Min::accept(Visitor& v)        { v.visit(*this); }
  void Max::accept(Visitor& v)        { v.visit(*this); }
  void Map::accept(Visitor& v)        { v.visit(*this); }
  void Reduce::accept(Visitor& v)     { v.visit(*this); }
  void Filter::accept(Visitor& v)     { v.visit(*this); }
  void All::accept(Visitor& v)        { v.visit(*this); }
  void None::accept(Visitor& v)       { v.visit(*this); }
  void Some::accept(Visitor& v)       { v.visit(*this); }
  void Array::accept(Visitor& v)      { v.visit(*this); }
  void Merge::accept(Visitor& v)      { v.visit(*this); }
  void Cat::accept(Visitor& v)        { v.visit(*this); }
  void Substr::accept(Visitor& v)     { v.visit(*this); }
  void In::accept(Visitor& v)         { v.visit(*this); }
  void Var::accept(Visitor& v)        { v.visit(*this); }
  void Log::accept(Visitor& v)        { v.visit(*this); }
  void If::accept(Visitor& v)         { v.visit(*this); }

  void NullVal::accept(Visitor& v)    { v.visit(*this); }
  void BoolVal::accept(Visitor& v)    { v.visit(*this); }
  void IntVal::accept(Visitor& v)     { v.visit(*this); }
  void UintVal::accept(Visitor& v)    { v.visit(*this); }
  void DoubleVal::accept(Visitor& v)  { v.visit(*this); }
  void StringVal::accept(Visitor& v)  { v.visit(*this); }

  void Error::accept(Visitor& v)      { v.visit(*this); }
  void RegexMatch::accept(Visitor& v) { v.visit(*this); }

  // toJson implementations
  template <class T>
  JsonExpr ValueT<T>::toJson() const { return value(); }

  JsonExpr NullVal::toJson()   const { return value(); }

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

  struct FwdVisitor : Visitor
  {
    void visit(Expr&)         override {} // error
    void visit(Operator& n)   override { visit(up_cast<Expr>(n)); }
    void visit(Eq& n)         override { visit(up_cast<Operator>(n)); }
    void visit(StrictEq& n)   override { visit(up_cast<Operator>(n)); }
    void visit(Neq& n)        override { visit(up_cast<Operator>(n)); }
    void visit(StrictNeq& n)  override { visit(up_cast<Operator>(n)); }
    void visit(Less& n)       override { visit(up_cast<Operator>(n)); }
    void visit(Greater& n)    override { visit(up_cast<Operator>(n)); }
    void visit(Leq& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Geq& n)        override { visit(up_cast<Operator>(n)); }
    void visit(And& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Or& n)         override { visit(up_cast<Operator>(n)); }
    void visit(Not& n)        override { visit(up_cast<Operator>(n)); }
    void visit(NotNot& n)     override { visit(up_cast<Operator>(n)); }
    void visit(Add& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Sub& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Mul& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Div& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Mod& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Min& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Max& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Map& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Reduce& n)     override { visit(up_cast<Operator>(n)); }
    void visit(Filter& n)     override { visit(up_cast<Operator>(n)); }
    void visit(All& n)        override { visit(up_cast<Operator>(n)); }
    void visit(None& n)       override { visit(up_cast<Operator>(n)); }
    void visit(Some& n)       override { visit(up_cast<Operator>(n)); }
    void visit(Array& n)      override { visit(up_cast<Operator>(n)); }
    void visit(Merge& n)      override { visit(up_cast<Operator>(n)); }
    void visit(Cat& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Substr& n)     override { visit(up_cast<Operator>(n)); }
    void visit(In& n)         override { visit(up_cast<Operator>(n)); }
    void visit(Var& n)        override { visit(up_cast<Operator>(n)); }
    void visit(Log& n)        override { visit(up_cast<Operator>(n)); }

    void visit(If& n)         override { visit(up_cast<Expr>(n)); }

    void visit(Value& n)      override { visit(up_cast<Expr>(n)); }
    void visit(NullVal& n)    override { visit(up_cast<Value>(n)); }
    void visit(BoolVal& n)    override { visit(up_cast<Value>(n)); }
    void visit(IntVal& n)     override { visit(up_cast<Value>(n)); }
    void visit(UintVal& n)    override { visit(up_cast<Value>(n)); }
    void visit(DoubleVal& n)  override { visit(up_cast<Value>(n)); }
    void visit(StringVal& n)  override { visit(up_cast<Value>(n)); }

    void visit(Error& n)      override { visit(up_cast<Expr>(n)); }

    // extensions
    void visit(RegexMatch& n) override { visit(up_cast<Expr>(n)); }
  };

  template <class UnderVisitorT>
  struct GVisitor : FwdVisitor
  {
      explicit
      GVisitor(UnderVisitorT& selfref)
      : self(selfref)
      {}

      // list of all concrete types
      void visit(Eq& n)         final { self.gvisit(n); }
      void visit(StrictEq& n)   final { self.gvisit(n); }
      void visit(Neq& n)        final { self.gvisit(n); }
      void visit(StrictNeq& n)  final { self.gvisit(n); }
      void visit(Less& n)       final { self.gvisit(n); }
      void visit(Greater& n)    final { self.gvisit(n); }
      void visit(Leq& n)        final { self.gvisit(n); }
      void visit(Geq& n)        final { self.gvisit(n); }
      void visit(And& n)        final { self.gvisit(n); }
      void visit(Or& n)         final { self.gvisit(n); }
      void visit(Not& n)        final { self.gvisit(n); }
      void visit(NotNot& n)     final { self.gvisit(n); }
      void visit(Add& n)        final { self.gvisit(n); }
      void visit(Sub& n)        final { self.gvisit(n); }
      void visit(Mul& n)        final { self.gvisit(n); }
      void visit(Div& n)        final { self.gvisit(n); }
      void visit(Mod& n)        final { self.gvisit(n); }
      void visit(Min& n)        final { self.gvisit(n); }
      void visit(Max& n)        final { self.gvisit(n); }
      void visit(Map& n)        final { self.gvisit(n); }
      void visit(Reduce& n)     final { self.gvisit(n); }
      void visit(Filter& n)     final { self.gvisit(n); }
      void visit(All& n)        final { self.gvisit(n); }
      void visit(None& n)       final { self.gvisit(n); }
      void visit(Some& n)       final { self.gvisit(n); }
      void visit(Array& n)      final { self.gvisit(n); }
      void visit(Merge& n)      final { self.gvisit(n); }
      void visit(Cat& n)        final { self.gvisit(n); }
      void visit(Substr& n)     final { self.gvisit(n); }
      void visit(In& n)         final { self.gvisit(n); }
      void visit(Var& n)        final { self.gvisit(n); }
      void visit(Log& n)        final { self.gvisit(n); }

      void visit(If& n)         final { self.gvisit(n); }

      void visit(Value& n)      final { self.gvisit(n); }
      void visit(NullVal& n)    final { self.gvisit(n); }
      void visit(BoolVal& n)    final { self.gvisit(n); }
      void visit(IntVal& n)     final { self.gvisit(n); }
      void visit(UintVal& n)    final { self.gvisit(n); }
      void visit(DoubleVal& n)  final { self.gvisit(n); }
      void visit(StringVal& n)  final { self.gvisit(n); }

      void visit(Error& n)      final { self.gvisit(n); }
      void visit(RegexMatch& n) final { self.gvisit(n); }

    private:
      UnderVisitorT& self;
  };


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
      throw type_error("typing error");
    }

    CXX_NORETURN
    void requiresArgumentError()
    {
      throw std::runtime_error("insufficient arguments");
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
      try
      {
        AnyExpr&   arg  = var.back();
        StringVal& str  = down_cast<StringVal>(*arg);
        const bool comp = (  str.value().find('.') != json::string::npos
                          && str.value().find('[') != json::string::npos
                          );

        if (comp)
        {
          hasComputed = true;
        }
        else if (str.value() != "") // do nothing for free variables in "lambdas"
        {
          auto [pos, success] = mapping.emplace(str.value(), mapping.size());

          var.num(pos->second);
        }
      }
      catch (const cast_error&)
      {
        hasComputed = true;
      }
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
    ExprT& mkOperator_(json::object& n, VarMap& m)
    {
      assert(n.size() == 1);

      ExprT& res = deref(new ExprT);

      res.set_operands(translateChildren(n.begin()->value(), m));
      return res;
    }

    template <class ExprT>
    Expr& mkOperator(json::object& n, VarMap& m)
    {
      return mkOperator_<ExprT>(n, m);
    }

    Expr& mkVar(json::object& n, VarMap& m)
    {
      Var& v = mkOperator_<Var>(n, m);

      m.insert(v);
      return v;
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
                                        { "var",    &mkVar },
                                        /// extensions
                                        { "regex",  &mkOperator<RegexMatch> },
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

    inline
    std::tuple<AnyExpr, std::vector<json::string>, bool>
    translateNode(JsonExpr& n)
    {
      VarMap  varmap;
      AnyExpr node = translateNode_internal(n, varmap);
      bool    hasComputedVariables = varmap.hasComputedVariables();

      return { std::move(node), varmap.toVector(), hasComputedVariables };
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

  std::ostream& operator<<(std::ostream& os, ValueExpr& n);

  ValueExpr toValueExpr(std::nullptr_t)    { return ValueExpr(new NullVal); }
  ValueExpr toValueExpr(bool val)          { return ValueExpr(new BoolVal(val)); }
  ValueExpr toValueExpr(std::int64_t val)  { return ValueExpr(new IntVal(val)); }
  ValueExpr toValueExpr(std::uint64_t val) { return ValueExpr(new UintVal(val)); }
  ValueExpr toValueExpr(double val)        { return ValueExpr(new DoubleVal(val)); }
  ValueExpr toValueExpr(json::string val)  { return ValueExpr(new StringVal(std::move(val))); }

  CXX_MAYBE_UNUSED
  ValueExpr toValueExpr(const json::value& n)
  {
    ValueExpr res;

    switch (n.kind())
    {
      case json::kind::string:
        {
          res = toValueExpr(n.get_string());
          break;
        }

      case json::kind::int64:
        {
          res = toValueExpr(n.get_int64());
          break;
        }

      case json::kind::uint64:
        {
          res = toValueExpr(n.get_uint64());
          break;
        }

      case json::kind::double_:
        {
          res = toValueExpr(n.get_double());
          break;
        }

      case json::kind::bool_:
        {
          res = toValueExpr(n.get_bool());
          break;
        }

      case json::kind::null:
        {
          res = toValueExpr(nullptr);
          break;
        }

      default:
        unsupported();
    }

    assert(res.get());
    return res;
  }

  //
  // coercion functions

  struct CoercionError {};
  struct OutsideOfInt64Range  : CoercionError {};
  struct OutsideOfUint64Range : CoercionError {};
  struct UnpackedArrayRequired : CoercionError {};

  /// conversion to int64
  /// \{
  inline std::int64_t toConcreteValue(std::int64_t v, const std::int64_t&)          { return v; }
  inline std::int64_t toConcreteValue(const json::string& str, const std::int64_t&) { return std::stoll(std::string{str.c_str()}); }
  inline std::int64_t toConcreteValue(double v, const std::int64_t&)                { return v; }
  inline std::int64_t toConcreteValue(bool v, const std::int64_t&)                  { return v; }
  inline std::int64_t toConcreteValue(std::nullptr_t, const std::int64_t&)          { return 0; }

  inline
  std::int64_t toConcreteValue(std::uint64_t v, const std::int64_t&)
  {
    if (v > std::uint64_t(std::numeric_limits<std::int64_t>::max()))
    {
      CXX_UNLIKELY;
      throw OutsideOfInt64Range{};
    }

    return v;
  }
  /// \}

  /// conversion to uint64
  /// \{
  inline std::uint64_t toConcreteValue(std::uint64_t v, const std::uint64_t&)         { return v; }
  inline std::uint64_t toConcreteValue(const json::string& str, const std::uint64_t&) { return std::stoull(std::string{str.c_str()}); }
  inline std::uint64_t toConcreteValue(double v, const std::uint64_t&)                { return v; }
  inline std::uint64_t toConcreteValue(bool v, const std::uint64_t&)                  { return v; }
  inline std::uint64_t toConcreteValue(std::nullptr_t, const std::uint64_t&)          { return 0; }

  inline std::uint64_t toConcreteValue(std::int64_t v, const std::uint64_t&)
  {
    if (v < 0)
    {
      CXX_UNLIKELY;
      throw OutsideOfUint64Range{};
    }

    return v;
  }
  /// \}

  /// conversion to double
  /// \{
  inline double toConcreteValue(const json::string& str, const double&) { return std::stod(std::string{str.c_str()}); }
  inline double toConcreteValue(std::int64_t v, const double&)          { return v; }
  inline double toConcreteValue(std::uint64_t v, const double&)         { return v; }
  inline double toConcreteValue(double v, const double&)                { return v; }
  inline double toConcreteValue(bool v, const double&)                  { return v; }
  inline double toConcreteValue(std::nullptr_t, const double&)          { return 0; }
  /// \}

  /// conversion to string
  /// \{
  template <class Val>
  inline json::string toConcreteValue(Val v, const json::string&)                 { return json::string{std::to_string(v)}; }

  inline json::string toConcreteValue(bool v, const json::string&)                { return json::string{v ? "true" : "false"}; }
  inline json::string toConcreteValue(const json::string& s, const json::string&) { return s; }
  inline json::string toConcreteValue(std::nullptr_t, const json::string&)        { return json::string{"null"}; }
  /// \}


  /// conversion to boolean
  ///   implements truthy, falsy as described by https://jsonlogic.com/truthy.html
  /// \{
  inline bool toConcreteValue(bool v, const bool&)                { return v; }
  inline bool toConcreteValue(std::int64_t v, const bool&)        { return v; }
  inline bool toConcreteValue(std::uint64_t v, const bool&)       { return v; }
  inline bool toConcreteValue(double v, const bool&)              { return v; }
  inline bool toConcreteValue(const json::string& v, const bool&) { return v.size() != 0; }
  inline bool toConcreteValue(std::nullptr_t, const bool&)        { return false; }

  // \todo not sure if conversions from arrays to values should be supported like this
  inline bool toConcreteValue(const Array& v, const bool&)        { return v.num_evaluated_operands(); }
  /// \}

  struct ComparisonOperatorBase
  {
    enum
    {
      definedForString  = true,
      definedForDouble  = true,
      definedForInteger = true,
      definedForBool    = true,
      definedForNull    = true,
      definedForArray   = true
    };

    using result_type   = bool;
  };

  /// \brief a strict equality operator operates on operands of the same
  ///        type. The operation on two different types returns false.
  ///        NO type coercion is performed.
  struct StrictEqualityOperator : ComparisonOperatorBase
  {
    std::tuple<bool, bool>
    coerce(Array*, Array*)
    {
      return { true, false }; // arrays are never equal
    }

    template <class LhsT, class RhsT>
    std::tuple<LhsT, RhsT>
    coerce(LhsT* lv, RhsT* rv)
    {
      return { std::move(*lv), std::move(*rv) };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::nullptr_t)
    {
      return { nullptr, nullptr }; // two null pointers are equal
    }

    template <class LhsT>
    std::tuple<LhsT, std::nullptr_t>
    coerce(LhsT* lv, std::nullptr_t)
    {
      return { std::move(*lv), nullptr };
    }

    template <class RhsT>
    std::tuple<std::nullptr_t, RhsT>
    coerce(std::nullptr_t, RhsT* rv)
    {
      return { nullptr, std::move(*rv) };
    }
  };

  struct NumericBinaryOperatorBase
  {
    std::tuple<double, double>
    coerce(double* lv, double* rv)
    {
      return { *lv, *rv };
    }

    std::tuple<double, double>
    coerce(double* lv, std::int64_t* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<double, double>
    coerce(double* lv, std::uint64_t* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<double, double>
    coerce(std::int64_t* lv, double* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, std::int64_t* rv)
    {
      return { *lv, *rv };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, std::uint64_t* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<double, double>
    coerce(std::uint64_t* lv, double* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::uint64_t* lv, std::int64_t* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::uint64_t* lv, std::uint64_t* rv)
    {
      return { *lv, *rv };
    }
  };


  /// \brief an equality operator compares two values. If the
  ///        values have a different type, type coercion is performed
  ///        on one of the operands.
  struct RelationalOperatorBase : NumericBinaryOperatorBase
  {
    using NumericBinaryOperatorBase::coerce;

    std::tuple<double, double>
    coerce(double* lv, json::string* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<double, double>
    coerce(double* lv, bool* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, json::string* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, bool* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::uint64_t* lv, json::string* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::uint64_t* lv, bool* rv)
    {
      return { *lv, toConcreteValue(*rv, *lv) };
    }

    std::tuple<double, double>
    coerce(json::string* lv, double* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<double, double>
    coerce(bool* lv, double* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(json::string* lv, std::int64_t* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(bool* lv, std::int64_t* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(json::string* lv, std::uint64_t* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(bool* lv, std::uint64_t* rv)
    {
      return { toConcreteValue(*lv, *rv), *rv };
    }

    std::tuple<bool, bool>
    coerce(json::string*, bool* rv)
    {
      // strings and boolean are never equal
      return { !*rv, *rv };
    }

    std::tuple<bool, bool>
    coerce(bool* lv, json::string*)
    {
      // strings and boolean are never equal
      return { *lv, !*lv };
    }

    std::tuple<json::string, json::string>
    coerce(json::string* lv, json::string* rv)
    {
      return { std::move(*lv), std::move(*rv) };
    }

    std::tuple<bool, bool>
    coerce(bool* lv, bool* rv)
    {
      return { *lv, *rv };
    }
  };

  struct EqualityOperator   : RelationalOperatorBase, ComparisonOperatorBase
  {
    using RelationalOperatorBase::coerce;

    // due to special conversion rules, the coercion function may just produce
    //   the result instead of just unpacking and coercing values.

    std::tuple<bool, bool>
    coerce(Array*, Array*)
    {
      return { true, false }; // arrays are never equal
    }

    template <class T>
    std::tuple<bool, bool>
    coerce(T* lv, Array* rv)
    {
      // an array may be compared to a value
      //   (1) *lv == arr[0], iff the array has exactly one element
      if (rv->num_evaluated_operands() == 1)
        throw UnpackedArrayRequired{};

      //   (2) or if [] and *lv converts to false
      if (rv->num_evaluated_operands() > 1)
        return { false, true };

      const bool convToFalse = toConcreteValue(*lv, false) == false;

      return { convToFalse, true /* zero elements */ };
    }

    template <class T>
    std::tuple<bool, bool>
    coerce(Array* lv, T* rv)
    {
      // see comments in coerce(T*,Array*)
      if (lv->num_evaluated_operands() == 1)
        throw UnpackedArrayRequired{};

      if (lv->num_evaluated_operands() > 1)
        return { false, true };

      const bool convToFalse = toConcreteValue(*rv, false) == false;

      return { true /* zero elements */, convToFalse };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::nullptr_t)
    {
      return { nullptr, nullptr }; // two null pointers are equal
    }

    template <class T>
    std::tuple<bool, bool>
    coerce(T*, std::nullptr_t)
    {
      return { false, true }; // null pointer is only equal to itself
    }

    template <class T>
    std::tuple<bool, bool>
    coerce(std::nullptr_t, T*)
    {
      return { true, false }; // null pointer is only equal to itself
    }

    std::tuple<bool, bool>
    coerce(Array*, std::nullptr_t)
    {
      return { true, false }; // null pointer is only equal to itself
    }

    std::tuple<bool, bool>
    coerce(std::nullptr_t, Array*)
    {
      return { true, false }; // null pointer is only equal to itself
    }
  };

  struct RelationalOperator : RelationalOperatorBase, ComparisonOperatorBase
  {
    using RelationalOperatorBase::coerce;

    std::tuple<Array*, Array*>
    coerce(Array* lv, Array* rv)
    {
      return { lv, rv };
    }

    template <class T>
    std::tuple<bool, bool>
    coerce(T* lv, Array* rv)
    {
      // an array may be equal to another value if
      //   (1) *lv == arr[0], iff the array has exactly one element
      if (rv->num_evaluated_operands() == 1)
        throw UnpackedArrayRequired{};

      //   (2) or if [] and *lv converts to false
      if (rv->num_evaluated_operands() > 1)
        return { false, true };

      const bool convToTrue = toConcreteValue(*lv, true) == true;

      return { convToTrue, false /* zero elements */ };
    }

    template <class T>
    std::tuple<bool, bool>
    coerce(Array* lv, T* rv)
    {
      // see comments in coerce(T*,Array*)
      if (lv->num_evaluated_operands() == 1)
        throw UnpackedArrayRequired{};

      if (lv->num_evaluated_operands() > 1)
        return { false, true };

      const bool convToTrue = toConcreteValue(*rv, true) == true;

      return { false /* zero elements */, convToTrue };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::nullptr_t)
    {
      return { nullptr, nullptr }; // two null pointers are equal
    }

    std::tuple<bool, bool>
    coerce(bool* lv, std::nullptr_t)
    {
      return { *lv, false }; // null pointer -> false
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::int64_t* lv, std::nullptr_t)
    {
      return { *lv, 0 }; // null pointer -> 0
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::uint64_t* lv, std::nullptr_t)
    {
      return { *lv, 0 }; // null pointer -> 0
    }

    std::tuple<double, double>
    coerce(double* lv, std::nullptr_t)
    {
      return { *lv, 0 }; // null pointer -> 0.0
    }

    std::tuple<json::string, std::nullptr_t>
    coerce(json::string* lv, std::nullptr_t)
    {
      return { std::move(*lv), nullptr }; // requires special handling
    }

    std::tuple<bool, bool>
    coerce(std::nullptr_t, bool* rv)
    {
      return { false, *rv }; // null pointer -> false
    }

    std::tuple<std::int64_t, std::int64_t>
    coerce(std::nullptr_t, std::int64_t* rv)
    {
      return { 0, *rv }; // null pointer -> 0
    }

    std::tuple<std::uint64_t, std::uint64_t>
    coerce(std::nullptr_t, std::uint64_t* rv)
    {
      return { 0, *rv }; // null pointer -> 0
    }

    std::tuple<double, double>
    coerce(std::nullptr_t, double* rv)
    {
      return { 0, *rv }; // null pointer -> 0
    }

    std::tuple<std::nullptr_t, json::string>
    coerce(std::nullptr_t, json::string* rv)
    {
      return { nullptr, std::move(*rv) }; // requires special handling
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
      definedForNull    = true,
      definedForArray   = false
    };

    using result_type = ValueExpr;

    using NumericBinaryOperatorBase::coerce;

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(double*, std::nullptr_t)
    {
      return { nullptr, nullptr };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::int64_t*, std::nullptr_t)
    {
      return { nullptr, nullptr };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::uint64_t*, std::nullptr_t)
    {
      return { nullptr, nullptr };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, double*)
    {
      return { nullptr, nullptr };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::int64_t*)
    {
      return { nullptr, nullptr };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::uint64_t*)
    {
      return { nullptr, nullptr };
    }

    std::tuple<std::nullptr_t, std::nullptr_t>
    coerce(std::nullptr_t, std::nullptr_t)
    {
      return { nullptr, nullptr };
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
      definedForNull    = false,
      definedForArray   = false
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
      definedForNull    = false,
      definedForArray   = false
    };

    using result_type = ValueExpr;

    std::tuple<json::string, json::string>
    coerce(json::string* lv, json::string* rv)
    {
      return { std::move(*lv), std::move(*rv) };
    }
  };

  struct ArrayOperator
  {
    enum
    {
      definedForString  = false,
      definedForDouble  = false,
      definedForInteger = false,
      definedForBool    = false,
      definedForNull    = false,
      definedForArray   = true
    };

    using result_type = ValueExpr;

    std::tuple<Array*, Array*>
    coerce(Array* lv, Array* rv)
    {
      return { lv, rv };
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
          double       dd = toConcreteValue(el.value(), double{});
          std::int64_t ii = toConcreteValue(el.value(), std::int64_t{});
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
          res = toValueExpr(toConcreteValue(el.value(), std::int64_t{}));
        }

        void visit(BoolVal& el) final
        {
          res = toValueExpr(toConcreteValue(el.value(), std::int64_t{}));
        }

        void visit(DoubleVal& el) final
        {
          res = toValueExpr(toConcreteValue(el.value(), std::int64_t{}));
        }

        void visit(NullVal&)      final
        {
          res = toValueExpr(std::int64_t{0});
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
          res = toValueExpr(toConcreteValue(el.value(), json::string{}));
        }

        void visit(IntVal& el)    final
        {
          res = toValueExpr(toConcreteValue(el.value(), json::string{}));
        }

        void visit(UintVal& el)   final
        {
          res = toValueExpr(toConcreteValue(el.value(), json::string{}));
        }

        void visit(DoubleVal& el) final
        {
          res = toValueExpr(toConcreteValue(el.value(), json::string{}));
        }

        void visit(NullVal& el)      final
        {
          res = toValueExpr(toConcreteValue(el.value(), json::string{}));
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

  AnyExpr convert(AnyExpr val, const ArrayOperator&)
  {
    struct ArrayConverter : FwdVisitor
    {
        explicit
        ArrayConverter(AnyExpr val)
        : val(std::move(val))
        {}

        void visit(Expr&)         final { typeError(); }

        // moves res into
        void toArray()
        {
          Array&                   arr = deref(new Array);
          Operator::container_type operands;

          operands.emplace_back(&arr);

          // swap the operand and result
          std::swap(operands.back(), val);

          // then set the operands
          arr.set_operands(std::move(operands));
        }

        // defined for the following types
        void visit(Array&)     final {}

        // need to move value to new array
        void visit(StringVal&) final { toArray(); }
        void visit(BoolVal&)   final { toArray(); }
        void visit(IntVal&)    final { toArray(); }
        void visit(UintVal&)   final { toArray(); }
        void visit(DoubleVal&) final { toArray(); }
        void visit(NullVal&)   final { toArray(); }

        AnyExpr result() && { return std::move(val); }

      private:
        AnyExpr val;
    };

    Expr*          node = val.get();
    ArrayConverter conv{std::move(val)};

    node->accept(conv);
    return std::move(conv).result();
  }


  template <class ValueT>
  struct UnpackVisitor : FwdVisitor
  {
      UnpackVisitor()
      : res()
      {}

      void assign(ValueT& lhs, const ValueT& val) { lhs = val; }

      template <class U>
      void assign(ValueT& lhs, const U& val)
      {
        lhs = toConcreteValue(val, lhs);
      }

      void visit(Expr&) final { typeError(); }

      // defined for the following types
      void visit(StringVal& el)  final
      {
        assign(res, el.value());
      }

      // need to convert values
      void visit(BoolVal& el) final
      {
        assign(res, el.value());
      }

      void visit(IntVal& el) final
      {
        assign(res, el.value());
      }

      void visit(UintVal& el) final
      {
        assign(res, el.value());
      }

      void visit(DoubleVal& el) final
      {
        assign(res, el.value());
      }

      void visit(NullVal& el) final
      {
        assign(res, el.value());
      }

      void visit(Array& el) final
      {
        if constexpr (std::is_same<ValueT, bool>::value)
          return assign(res, el);

        typeError();
      }

      ValueT result() && { return std::move(res); }

    private:
      ValueT res;
  };


  template <class T>
  T unpackValue(Expr& expr)
  {
    UnpackVisitor<T> unpack;

    expr.accept(unpack);
    return std::move(unpack).result();
  }

  template <class T>
  T unpackValue(ValueExpr& el)
  {
    return unpackValue<T>(*el);
  }

  template <class T>
  T unpackValue(ValueExpr&& el)
  {
    return unpackValue<T>(*el);
  }

  template <class T>
  bool toBool(ValueExpr& el)
  {
    return unpackValue<bool>(el);
  }

  template <class T>
  bool toBool(ValueExpr&& el)
  {
    return unpackValue<bool>(el);
  }

  template <class ExprT>
  struct DownCastVisitor : GVisitor<DownCastVisitor<ExprT> >
  {
      using base = GVisitor<DownCastVisitor<ExprT> >;

      DownCastVisitor()
      : base(*this), res(nullptr)
      {}

      void gvisit(Expr&)    { /* not found */ }

      void gvisit(ExprT& n) { res = &n; }

      ExprT& result() { return deref<cast_error>(res); }

    private:
      ExprT* res;
  };


  template <class ExprT>
  ExprT& down_cast(Expr& expr)
  {
    DownCastVisitor<ExprT> vis;

    expr.accept(vis);
    return vis.result();
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

        res = op(std::move(ll), std::move(rr));
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
        {
          try
          {
            return calc(&n.value());
          }
          catch (const OutsideOfInt64Range& ex)
          {
            if (n.value() < 0)
            {
              CXX_UNLIKELY;
              throw std::range_error{"unable to consolidate uint>max(int) with int<0"};
            }
          }

          std::uint64_t alt = n.value();
          return calc(&alt);
        }

        typeError();
      }

      void visit(UintVal& n) final
      {
        if constexpr (BinaryOperator::definedForInteger)
        {
          try
          {
            return calc(&n.value());
          }
          catch (const OutsideOfUint64Range& ex)
          {
            if (n.value() > std::uint64_t(std::numeric_limits<std::int64_t>::max))
            {
              CXX_UNLIKELY;
              throw std::range_error{"unable to consolidate int<0 with uint>max(int)"};
            }
          }

          std::int64_t alt = n.value();
          return calc(&alt);
        }

        typeError();
      }

      void visit(DoubleVal& n) final
      {
        if constexpr (BinaryOperator::definedForDouble)
          return calc(&n.value());

        typeError();
      }

      void visit(Array& n) final
      {
        if constexpr (BinaryOperator::definedForArray)
        {
          try
          {
            calc(&n);
          }
          catch (const UnpackedArrayRequired&)
          {
            assert(n.num_evaluated_operands() == 1);
            n.operand(0).accept(*this);
          }

          return;
        }

        typeError();
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
        {
          try
          {
            return calc(&n.value());
          }
          catch (const OutsideOfInt64Range& ex)
          {
            if (n.value() < 0)
            {
              CXX_UNLIKELY;
              throw std::range_error{"unable to consolidate int<0 with uint>max(int)"};
            }
          }

          std::uint64_t alt = n.value();
          return calc(&alt);
        }

        typeError();
      }

      void visit(UintVal& n) final
      {
        if constexpr (BinaryOperator::definedForInteger)
        {
          try
          {
            return calc(&n.value());
          }
          catch (const OutsideOfUint64Range& ex)
          {
            if (n.value() > std::uint64_t(std::numeric_limits<std::int64_t>::max))
            {
              CXX_UNLIKELY;
              throw std::range_error{"unable to consolidate uint>max(int) with int<0"};
            }
          }

          std::int64_t alt = n.value();
          return calc(&alt);
        }

        typeError();
      }

      void visit(DoubleVal& n) final
      {
        if constexpr (BinaryOperator::definedForDouble)
          return calc(&n.value());

        typeError();
      }

      void visit(Array& n) final
      {
        if constexpr (BinaryOperator::definedForArray)
        {
          try
          {
            calc(&n);
          }
          catch (const UnpackedArrayRequired&)
          {
            assert(n.num_evaluated_operands() == 1);
            n.operand(0).accept(*this);
          }

          return;
        }

        typeError();
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

  template <class BinaryPredicate>
  bool compareSeq(Array& lv, Array& rv, BinaryPredicate pred)
  {
    const std::size_t lsz = lv.num_evaluated_operands();
    const std::size_t rsz = rv.num_evaluated_operands();

    if (lsz == 0)
      return pred( false, rsz != 0 );

    if (rsz == 0)
      return pred( true, false );

    std::size_t const len   = std::min(lsz, rsz);
    std::size_t       i     = 0;
    bool              res   = false;
    bool              found = false;

    while ((i < len) && !found)
    {
      res   = compute(lv.at(i), rv.at(i), pred);

      // res is conclusive if the reverse test yields a different result
      found = res != compute(rv.at(i), lv.at(i), pred);

      ++i;
    }

    return found ? res : pred(lsz, rsz);
  }


  template <class BinaryPredicate>
  bool compareSeq(Array* lv, Array* rv, BinaryPredicate pred)
  {
    return compareSeq(deref(lv), deref(rv), std::move(pred));
  }


  template <class>
  struct Calc {};

  template <>
  struct Calc<Eq> : EqualityOperator
  {
    using EqualityOperator::result_type;

    result_type operator()(...) const { return false; } // type mismatch

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs == rhs;
    }
  };

  template <>
  struct Calc<Neq> : EqualityOperator
  {
    using EqualityOperator::result_type;

    result_type operator()(...) const { return true; } // type mismatch

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs != rhs;
    }
  };

  template <>
  struct Calc<StrictEq> : StrictEqualityOperator
  {
    using StrictEqualityOperator::result_type;

    result_type operator()(...) const { return false; } // type mismatch

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs == rhs;
    }
  };

  template <>
  struct Calc<StrictNeq> : StrictEqualityOperator
  {
    using StrictEqualityOperator::result_type;

    result_type operator()(...) const { return true; } // type mismatch

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs != rhs;
    }
  };

  template <>
  struct Calc<Less> : RelationalOperator
  {
    using RelationalOperator::result_type;

    result_type
    operator()(const std::nullptr_t, std::nullptr_t) const
    {
      return false;
    }

    result_type
    operator()(Array* lv, Array* rv) const
    {
      return compareSeq(lv, rv, *this);
    }

    result_type
    operator()(const json::string&, std::nullptr_t) const
    {
      return false;
    }

    result_type
    operator()(std::nullptr_t, const json::string&) const
    {
      return false;
    }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs < rhs;
    }
  };

  template <>
  struct Calc<Greater> : RelationalOperator
  {
    using RelationalOperator::result_type;

    result_type
    operator()(const std::nullptr_t, std::nullptr_t) const
    {
      return false;
    }

    result_type
    operator()(Array* lv, Array* rv) const
    {
      return compareSeq(lv, rv, *this);
    }

    result_type
    operator()(const json::string&, std::nullptr_t) const
    {
      return false;
    }

    result_type
    operator()(std::nullptr_t, const json::string&) const
    {
      return false;
    }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return rhs < lhs;
    }
  };

  template <>
  struct Calc<Leq> : RelationalOperator
  {
    using RelationalOperator::result_type;

    result_type
    operator()(const std::nullptr_t, std::nullptr_t) const
    {
      return true;
    }

    result_type
    operator()(Array* lv, Array* rv) const
    {
      return compareSeq(lv, rv, *this);
    }

    result_type
    operator()(const json::string& lhs, std::nullptr_t) const
    {
      return lhs.empty();
    }

    result_type
    operator()(std::nullptr_t, const json::string& rhs) const
    {
      return rhs.empty();
    }

    template <class T>
    result_type
    operator()(const T& lhs, const T& rhs) const
    {
      return lhs <= rhs;
    }
  };

  template <>
  struct Calc<Geq> : RelationalOperator
  {
    using RelationalOperator::result_type;

    result_type
    operator()(const std::nullptr_t, std::nullptr_t) const
    {
      return true;
    }

    result_type
    operator()(Array* lv, Array* rv) const
    {
      return compareSeq(lv, rv, *this);
    }

    result_type
    operator()(const json::string& lhs, std::nullptr_t) const
    {
      return lhs.empty();
    }

    result_type
    operator()(std::nullptr_t, const json::string& rhs) const
    {
      return rhs.empty();
    }

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

    result_type
    operator()(const std::nullptr_t, std::nullptr_t) const
    {
      return nullptr;
    }

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

    result_type
    operator()(const std::nullptr_t, std::nullptr_t) const
    {
      return nullptr;
    }

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
      return !unpackValue<bool>(val);
    }
  };

  template <>
  struct Calc<NotNot>
  {
    using result_type = bool;

    result_type
    operator()(Expr& val) const
    {
      return unpackValue<bool>(val);
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

  template <>
  struct Calc<In> : StringOperator // \todo the conversion rules differ
  {
    using StringOperator::result_type;

    result_type
    operator()(const json::string& lhs, const json::string& rhs) const
    {
      const bool res = (rhs.find(lhs) != json::string::npos);

      return toValueExpr(res);
    }
  };

  template <>
  struct Calc<RegexMatch> : StringOperator // \todo the conversion rules differ
  {
    using StringOperator::result_type;

    result_type
    operator()(const json::string& lhs, const json::string& rhs) const
    {
      std::regex rgx(lhs.c_str(), lhs.size());

      return toValueExpr(std::regex_search(rhs.begin(), rhs.end(), rgx));
    }
  };

  template <>
  struct Calc<Merge> : ArrayOperator
  {
    using ArrayOperator::result_type;

    result_type
    operator()(Array* lhs, Array* rhs) const
    {
      // note, to use the lhs entirely, it would need to be released
      //   from its  ValueExpr
      Array& res = deref(new Array);

      {
        Operator::container_type& opers = res.operands();

        opers.swap(lhs->operands());

        Operator::container_type& ropers = rhs->operands();

        opers.insert( opers.end(),
                      std::make_move_iterator(ropers.begin()), std::make_move_iterator(ropers.end())
                    );
      }

      return ValueExpr(&res);
    }
  };

  struct Calculator : FwdVisitor
  {
      using VarAccess = std::function<ValueExpr(const json::value&, int)>;

      Calculator(VarAccess varAccess, std::ostream& out)
      : vars(std::move(varAccess)), logger(out), calcres(nullptr)
      {}

      void visit(Eq&)           final;
      void visit(StrictEq&)     final;
      void visit(Neq&)          final;
      void visit(StrictNeq&)    final;
      void visit(Less&)         final;
      void visit(Greater&)      final;
      void visit(Leq&)          final;
      void visit(Geq&)          final;
      void visit(And&)          final;
      void visit(Or&)           final;
      void visit(Not&)          final;
      void visit(NotNot&)       final;
      void visit(Add&)          final;
      void visit(Sub&)          final;
      void visit(Mul&)          final;
      void visit(Div&)          final;
      void visit(Mod&)          final;
      void visit(Min&)          final;
      void visit(Max&)          final;
      void visit(Array&)        final;
      void visit(Map&)          final;
      void visit(Reduce&)       final;
      void visit(Filter&)       final;
      void visit(All&)          final;
      void visit(None&)         final;
      void visit(Some&)         final;
      void visit(Merge&)        final;
      void visit(Cat&)          final;
      void visit(Substr&)       final;
      void visit(In&)           final;
      void visit(Var&)          final;
      void visit(Log&)          final;

      void visit(If&)           final;

      void visit(NullVal& n)    final;
      void visit(BoolVal& n)    final;
      void visit(IntVal& n)     final;
      void visit(UintVal& n)    final;
      void visit(DoubleVal& n)  final;
      void visit(StringVal& n)  final;

      void visit(Error& n)      final;
      void visit(RegexMatch& n) final;

      ValueExpr eval(Expr& n);

    private:
      VarAccess       vars;
      std::ostream&   logger;
      ValueExpr       calcres;

      Calculator(const Calculator&)            = delete;
      Calculator(Calculator&&)                 = delete;
      Calculator& operator=(const Calculator&) = delete;
      Calculator& operator=(Calculator&&)      = delete;

      //
      // opers

      /// implements relop : [1, 2, 3, whatever] as 1 relop 2 relop 3
      template <class BinaryPredicate>
      void evalPairShortCircuit(Operator& n, BinaryPredicate pred);

      /// returns the first expression in [ e1, e2, e3 ] that evaluates to val, or the last expression otherwise
      void evalShortCircuit(Operator& n, bool val);

      /// reduction operation on all elements
      template <class BinaryOperator>
      void reduce(Operator& n, BinaryOperator op);

      /// computes unary operation on n[0]
      template <class UnaryOperator>
      void unary(Operator& n, UnaryOperator calc);

      /// binary operation on all elements (invents an element if none is present)
      template <class BinaryOperator>
      void binary(Operator& n, BinaryOperator binop);

      /// evaluates and unpacks n[argpos] to a fundamental value
      template <class ValueT>
      ValueT unpackOptionalArg(Operator& n, int argpos, const ValueT& defaultVal);

      template <class ValueNode>
      void _value(const ValueNode& val)
      {
        calcres = toValueExpr(val.value());
      }
  };

  struct SequencePredicate
  {
      SequencePredicate(Expr& e, std::ostream& logstream)
      : expr(e), logger(logstream)
      {}

      bool operator()(ValueExpr&& elem) const
      {
        ValueExpr* elptr = &elem; // workaround, since I am unable to capture a unique_ptr

        Calculator sub{ [elptr]
                        (const json::value& keyval, int) mutable -> ValueExpr
                        {
                          if (const json::string* pkey = keyval.if_string())
                          {
                            const json::string& key = *pkey;

                            if (key.size() == 0) return std::move(*elptr);

                            try
                            {
                              ObjectVal& o = down_cast<ObjectVal>(**elptr);

                              if (auto pos = o.find(key); pos != o.end())
                                return std::move(pos->second);
                            }
                            catch (const cast_error&) {}
                          }


                          return toValueExpr(nullptr);
                        },
                        logger
                      };

        return unpackValue<bool>(sub.eval(expr));
      }

    private:
      Expr&         expr;
      std::ostream& logger;
  };


  template <class ValueT>
  ValueT
  Calculator::unpackOptionalArg(Operator& n, int argpos, const ValueT& defaultVal)
  {
    if (std::size_t(argpos) >= n.size())
    {
      CXX_UNLIKELY;
      return defaultVal;
    }

    return unpackValue<ValueT>(*eval(n.operand(argpos)));
  }

  template <class UnaryPredicate>
  void
  Calculator::unary(Operator& n, UnaryPredicate pred)
  {
    const int  num = n.num_evaluated_operands();
    assert(num == 1);

    const bool res = pred(*eval(n.operand(0)));

    calcres = toValueExpr(res);
  }

  template <class BinaryOperator>
  void
  Calculator::binary(Operator& n, BinaryOperator binop)
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

    calcres = compute(lhs, rhs, binop);
  }

  template <class BinaryOperator>
  void
  Calculator::reduce(Operator& n, BinaryOperator op)
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

  template <class BinaryPredicate>
  void
  Calculator::evalPairShortCircuit(Operator& n, BinaryPredicate pred)
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

      res = compute(lhs, rhs, pred);
    }

    calcres = toValueExpr(res);
  }


  void
  Calculator::evalShortCircuit(Operator& n, bool val)
  {
    const int      num = n.num_evaluated_operands();

    if (num == 0) { CXX_UNLIKELY; requiresArgumentError(); }

    int            idx   = -1;
    ValueExpr      oper  = eval(n.operand(++idx));
    //~ std::cerr << idx << ") " << oper << std::endl;

    bool           found = (idx == num-1) || (unpackValue<bool>(*oper) == val);

    // loop until *aa == val or when *aa is the last valid element
    while (!found)
    {
      oper = eval(n.operand(++idx));
      //~ std::cerr << idx << ") " << oper << std::endl;

      found = (idx == (num-1)) || (unpackValue<bool>(*oper) == val);
    }

    calcres = std::move(oper);
  }

  ValueExpr
  Calculator::eval(Expr& n)
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

  void Calculator::visit(In& n)
  {
    binary(n, Calc<In>{});
  }

  void Calculator::visit(RegexMatch& n)
  {
    binary(n, Calc<In>{});
  }

  void Calculator::visit(Substr& n)
  {
    assert(n.num_evaluated_operands() >= 1);

    json::string str = unpackValue<json::string>(*eval(n.operand(0)));
    std::int64_t ofs = unpackOptionalArg<std::int64_t>(n, 1, 0);
    std::int64_t cnt = unpackOptionalArg<std::int64_t>(n, 2, 0);

    if (ofs < 0)
    {
      CXX_UNLIKELY;
      ofs = std::max(std::int64_t(str.size()) + ofs, std::int64_t(0));
    }

    if (cnt < 0)
    {
      CXX_UNLIKELY;
      cnt = std::max(std::int64_t(str.size()) - ofs + cnt, std::int64_t(0));
    }

    calcres = toValueExpr(json::string{str.subview(ofs, cnt)});
  }

  void Calculator::visit(Array& n)
  {
    Operator::container_type elems;
    Calculator*              self = this;

    // \todo consider making arrays lazy
    std::transform( std::make_move_iterator(n.begin()), std::make_move_iterator(n.end()),
                  std::back_inserter(elems),
                  [self](AnyExpr&& exp) -> ValueExpr
                  {
                    return self->eval(*exp);
                  }
                );

    Array& res = deref(new Array);

    res.set_operands(std::move(elems));

    calcres = ValueExpr(&res);
  }

  void Calculator::visit(Merge& n)
  {
    reduce(n, Calc<Merge>{});
  }

  void Calculator::visit(Map&)            { unsupported(); }
  void Calculator::visit(Reduce&)         { unsupported(); }
  void Calculator::visit(Filter&)         { unsupported(); }

  void Calculator::visit(All& n)
  {
    ValueExpr  arr   = eval(n.operand(0));
    Array&     elems = down_cast<Array>(*arr); // evaluated elements
    Expr&      expr  = n.operand(1);
    const bool res   = std::all_of( std::make_move_iterator(elems.begin()), std::make_move_iterator(elems.end()),
                                    SequencePredicate{expr, logger}
                                  );

    calcres = toValueExpr(res);
  }

  void Calculator::visit(None& n)
  {
    ValueExpr  arr   = eval(n.operand(0));
    Array&     elems = down_cast<Array>(*arr); // evaluated elements
    Expr&      expr  = n.operand(1);
    const bool res   = std::none_of( std::make_move_iterator(elems.begin()), std::make_move_iterator(elems.end()),
                                     SequencePredicate{expr, logger}
                                   );

    calcres = toValueExpr(res);
  }


  void Calculator::visit(Some& n)
  {
    ValueExpr  arr   = eval(n.operand(0));
    Array&     elems = down_cast<Array>(*arr); // evaluated elements
    Expr&      expr  = n.operand(1);
    const bool res   = std::any_of( std::make_move_iterator(elems.begin()), std::make_move_iterator(elems.end()),
                                    SequencePredicate{expr, logger}
                                  );

    calcres = toValueExpr(res);
  }

  void Calculator::visit(Error&) { unsupported(); }

  void Calculator::visit(Var& n)
  {
    assert(n.num_evaluated_operands() >= 1);

    AnyExpr elm = eval(n.operand(0));
    Value&  val = down_cast<Value>(*elm);

    try
    {
      calcres = vars(val.toJson(), n.num());
    }
    catch (...)
    {
      calcres = (n.num_evaluated_operands() > 1) ? eval(n.operand(1))
                                                 : toValueExpr(nullptr);
    }
  }

  void Calculator::visit(If& n)
  {
    const int num = n.num_evaluated_operands();

    if (num == 0)
    {
      calcres = toValueExpr(nullptr);
      return;
    }

    const int lim = num-1;
    int       pos = 0;

    while (pos < lim)
    {
      if (unpackValue<bool>(eval(n.operand(pos))))
      {
        calcres = eval(n.operand(pos+1));
        return;
      }

      pos+=2;
    }

    calcres = (pos < num) ? eval(n.operand(pos)) : toValueExpr(nullptr);
  }


  void Calculator::visit(Log& n)
  {
    const int num = n.num_evaluated_operands();
    assert(num == 1);

    calcres = eval(n.operand(0));

    logger << calcres << std::endl;
  }

  void Calculator::visit(NullVal& n)   { _value(n); }
  void Calculator::visit(BoolVal& n)   { _value(n); }
  void Calculator::visit(IntVal& n)    { _value(n); }
  void Calculator::visit(UintVal& n)   { _value(n); }
  void Calculator::visit(DoubleVal& n) { _value(n); }
  void Calculator::visit(StringVal& n) { _value(n); }

  CXX_MAYBE_UNUSED
  ValueExpr calculate(Expr& exp, const Calculator::VarAccess& vars)
  {
    Calculator calc{vars, std::cerr};

    return calc.eval(exp);
  }

  CXX_MAYBE_UNUSED
  ValueExpr calculate(AnyExpr& exp, const Calculator::VarAccess& vars)
  {
    assert(exp.get());
    return calculate(*exp, vars);
  }

  CXX_MAYBE_UNUSED
  ValueExpr calculate(AnyExpr& exp)
  {
    return calculate(exp, [](const json::value&, int) -> ValueExpr { unsupported(); });
  }


  ValueExpr evalPath(const json::string& path, const json::object& obj)
  {
    if (auto pos = obj.find(path); pos != obj.end())
      return json_logic::toValueExpr(pos->value());

    if (std::size_t pos = path.find('.'); pos != json::string::npos)
    {
      json::string selector = path.subview(0, pos);
      json::string suffix   = path.subview(pos+1);

      return evalPath(suffix, obj.at(selector).as_object());
    }

    throw std::out_of_range("json_logic - unable to locate path");
  }

  template <class IntT>
  ValueExpr evalIdx(IntT idx, const json::array& arr)
  {
    return json_logic::toValueExpr(arr[idx]);
  }


  CXX_MAYBE_UNUSED
  ValueExpr apply(json::value rule, json::value data)
  {
    auto [ast, vars, hasComputed] = translateNode(rule);
    Calculator::VarAccess varlookup =
                          [data]
                          (const json::value& keyval, int) -> ValueExpr
                          {
                            if (const json::string* ppath = keyval.if_string())
                            {
                              return ppath->size() ? evalPath(*ppath, data.as_object())
                                                   : json_logic::toValueExpr(data);
                            }

                            if (const std::int64_t* pidx = keyval.if_int64())
                              return evalIdx(*pidx, data.as_array());

                            if (const std::uint64_t* pidx = keyval.if_uint64())
                              return evalIdx(*pidx, data.as_array());

                            throw std::logic_error{"json_logic - unsupported var access"};
                          };

    return calculate(ast, varlookup);
  }

  struct ValuePrinter : FwdVisitor
  {
      explicit
      ValuePrinter(std::ostream& stream)
      : os(stream)
      {}

      void prn(json::value val)
      {
        os << val;
      }

      void visit(Value& n) final
      {
        prn(n.toJson());
      }

      void visit(Array& n) final
      {
        bool first = true;

        os << "[";
        for (AnyExpr& el : n)
        {
          if (first) first = false; else os << ", ";

          deref(el).accept(*this);
        }

        os << "]";
      }

    private:
      std::ostream& os;
  };

  std::ostream& operator<<(std::ostream& os, ValueExpr& n)
  {
    ValuePrinter prn{os};

    deref(n).accept(prn);
    return os;
  }
}


#if SUPPLEMENTAL

  /// traverses the children of a node; does not traverse grandchildren
  void traverseChildren(Visitor& v, const Operator& node);
  void traverseAllChildren(Visitor& v, const Operator& node);
  void traverseChildrenReverse(Visitor& v, const Operator& node);


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

  /// AST traversal function that calls v's visit methods in post-fix order
  void traverseInSAttributeOrder(Expr& e, Visitor& vis);

  void traverseInSAttributeOrder(Expr& e, Visitor& vis)
  {
    SAttributeTraversal trav{vis};

    e.accept(trav);
  }
#endif /* SUPPLEMENTAL */
