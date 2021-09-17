
#pragma once

#include <cassert>
#include <utility>
#include <memory>
// #include <span>
#include <variant>
//~ #include <map>

//~ 

#include <boost/container/vector.hpp>
#include <boost/container/string.hpp>
#include <boost/lexical_cast.hpp>

#include "metall/metall.hpp"
#include "metall/tags.hpp"

#include "experimental/flat_map.hpp"

namespace experimental
{
  
//~ using string_t = std::string;  
using string_t  = boost::container::string;  
using int_t     = int64_t;  
using uint_t    = uint64_t;  
using real_t    = double;  
using dataframe_variant_t = std::variant<int_t, real_t, uint_t, string_t>; 

struct runtime_type_error : std::runtime_error 
{
  using base = std::runtime_error;
  using base::base;
};

struct unknown_column_error : std::runtime_error 
{
  using base = std::runtime_error;
  using base::base;
};  


template <class T>
struct DefaultValue
{
    void default_value(T&& val)
    {
      defaultval = std::move(val);
    }
    
    const T& default_value() const
    {
      return defaultval;
    }
    
  private:
    T defaultval;  
};

template < class KeyT,
           class ElemT,
           class Compare = std::less<KeyT>,
           class Alloc = metall::manager::allocator_type<std::pair<KeyT, ElemT> >,
           template <class, class> class Vector = boost::container::vector
         >
struct sparse_column : flat_map<KeyT, ElemT, Compare, Alloc, Vector>, DefaultValue<ElemT>
{
  using defvalbase     = DefaultValue<ElemT>;
  using base           = flat_map<KeyT, ElemT, Compare, Alloc, Vector>;
  using key_compare    = typename base::key_compare;
  using allocator_type = typename base::allocator_type;
  using value_type     = typename base::value_type;
  using mapped_type    = typename base::mapped_type;

  explicit
  sparse_column(const key_compare& keycomp, const allocator_type& alloc = allocator_type())
  : base(alloc, keycomp), defvalbase()
  {}

  explicit
  sparse_column(const allocator_type& alloc)
  : base(alloc), defvalbase()
  {}
};



template < class ValT,
           class AllocT = metall::manager::allocator_type<ValT>,
           template <class, class> class Vector = boost::container::vector
         >
struct dense_column : Vector<ValT, AllocT>, DefaultValue<ValT>
{ 
  using defvalbase     = DefaultValue<ValT>;
  using base           = Vector<ValT, AllocT>;
  using allocator_type = typename base::allocator_type;
  using value_type     = typename base::value_type;
  
  explicit
  dense_column(const allocator_type& alloc)
  : base(alloc), defvalbase()
  {}   
};  

template <class T>
//~ using dense_vector_t  = boost::container::vector<T, metall::manager::allocator_type<T>>;
using dense_vector_t  = dense_column<T, metall::manager::allocator_type<T>>;


template <class T>
using sparse_vector_t = sparse_column<size_t, T>;
//~ using sparse_vector_t = boost::container::map<size_t, T, std::less<size_t>, metall::manager::allocator_type<std::pair<const size_t, T> >;


namespace
{

[[noreturn]]
inline
void errTypeMismatch(std::string cell = {}, std::string xpct = {})
{
  std::string err{"type mismatch:"};
  
  if (cell.size()) (err += " got ") += cell;
  if (xpct.size()) (err += " expected ") += xpct;
  
  throw runtime_type_error(err);
} 

template <class T>
constexpr 
const T* tag() { return nullptr; }

} // anonymous namespace

template <class ElemType>
struct AbstractColumnIterator
{
  using value_type = ElemType;
  
  virtual ~AbstractColumnIterator() {}
  virtual value_type& deref() = 0;
  virtual size_t row() const = 0;
  virtual void next() = 0;
  virtual void prev() = 0;
  virtual bool equals(const AbstractColumnIterator& other) const = 0;
  virtual AbstractColumnIterator* clone() const = 0;
};

namespace 
{

template <class T>
struct DenseColumnIterator : AbstractColumnIterator<T> 
{
    using base       = AbstractColumnIterator<T>;
    using ThisType   = DenseColumnIterator<T>;
    using value_type = typename base::value_type;
    using VectorRep  = dense_vector_t<T>;
    using Iterator   = typename VectorRep::iterator;
  
    DenseColumnIterator(Iterator pos, size_t rowcnt)
    : base(), it(pos), rownum(rowcnt)
    {}
    
    value_type& deref()       override { return *(this->it); }
    size_t      row()   const override { return this->rownum; }
    void        next()        override { ++(this->it); ++(this->rownum); } 
    void        prev()        override { --(this->it); --(this->rownum); } 
    
    bool equals(const base& other) const override
    {
      assert(typeid(*this) == typeid(other));
      
      const ThisType& rhs = static_cast<const ThisType&>(other);
      
      return this->it == rhs.it; // equal rownum is implied by equal iterators
    }
    
    ThisType* clone() const override 
    {
      return new ThisType(it, rownum);
    }
  
  private:
    Iterator it;
    size_t   rownum;
};

template <class T>
struct SparseColumnIterator : AbstractColumnIterator<T> 
{
    using base       = AbstractColumnIterator<T>;
    using ThisType   = SparseColumnIterator<T>;
    using value_type = typename base::value_type;
    using VectorRep  = sparse_vector_t<T>;
    using Iterator   = typename VectorRep::iterator;
  
    SparseColumnIterator(Iterator pos)
    : base(), it(pos)
    {}
    
    value_type& deref()     override { return this->it->second; }
    size_t      row() const override { return this->it->first; }
    void        next()      override { ++(this->it); } 
    void        prev()      override { --(this->it); } 
    
    bool equals(const base& other) const override
    {
      assert(typeid(*this) == typeid(other));
      
      const ThisType& rhs = static_cast<const ThisType&>(other);
      
      return this->it == rhs.it; // equal rownum is implied by equal iterators
    }
    
    ThisType* clone() const override 
    {
      return new ThisType(it);
    }
  
  private:
    Iterator it;
};

const std::string string_type_str{"string_t"}; 
const std::string int_type_str{"int_t"}; 
const std::string uint_type_str{"uint_t"}; 
const std::string real_type_str{"real_t"}; 

inline std::string to_string(const string_t*) { return string_type_str; }
inline std::string to_string(const int_t*)    { return int_type_str; }
inline std::string to_string(const uint_t*)   { return uint_type_str; }
inline std::string to_string(const real_t*)   { return real_type_str; }

template <class T>
inline std::string to_string(const T*) 
{ 
  return std::string{"unknown type; mangled name is: "} + typeid(T).name(); 
}

} // anonymous namespace

template <class T>
struct AnyColumnIterator 
{
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = T;
  using difference_type = int;
  using pointer = value_type*;
  using reference = value_type&;
  using ThisType = AnyColumnIterator<T>;
  
  explicit
  AnyColumnIterator(AbstractColumnIterator<T>* obj)
  : pit(obj)
  {}
  
  explicit
  AnyColumnIterator(typename dense_vector_t<T>::iterator it, size_t pos)
  : AnyColumnIterator(new DenseColumnIterator<T>(it, pos))
  {}
  
  explicit
  AnyColumnIterator(typename sparse_vector_t<T>::iterator it)
  : AnyColumnIterator(new SparseColumnIterator<T>(it))
  {}
  
  AnyColumnIterator(const AnyColumnIterator& other)
  : AnyColumnIterator(other.pit->clone())
  {}
  
  AnyColumnIterator(AnyColumnIterator&& other)
  : AnyColumnIterator(other.pit)
  {
    other.pit = nullptr;
  }
  
  AnyColumnIterator& operator=(const AnyColumnIterator& other)
  {
    AnyColumnIterator tmp(other.pit->clone());
    
    std::swap(tmp.pit, this->pit);
    return *this;
  }
  
  AnyColumnIterator& operator=(AnyColumnIterator&& other)
  {
    std::swap(other.pit, this->pit);
    return *this;
  }
  
  ~AnyColumnIterator()
  {
    delete pit;
  }
  
  
    
  T& operator*() { return pit->deref(); }
  size_t row() const { return pit->row(); }
  ThisType& operator++() { pit->next(); return *this; }
  
  ThisType operator++(int) 
  { 
    ThisType res{pit->clone()};
    
    pit->next(); 
    return *this; 
  }

  ThisType& operator--() { pit->prev(); return *this; }
  
  ThisType operator--(int) 
  { 
    ThisType res{pit->clone()};
    
    pit->prev(); 
    return *this; 
  }
  
  bool operator==(AnyColumnIterator& that) const
  {
    if (pit == nullptr || that.pit == nullptr)
      return pit == that.pit;

    return pit->equals(*(that.pit));
  }
  
  bool operator!=(AnyColumnIterator& that) const
  {
    return !(*this == that);
  }
  
  private:
    AbstractColumnIterator<T>* pit;
};


namespace
{
struct VectorAccessorRuntimeInfo 
{
  virtual
  std::string type_name() const = 0;
};


/// define operations on columns. 
template <class T>
struct VectorAccessorBaseT : VectorAccessorRuntimeInfo
{
  using entry_type = std::pair<const size_t, T>;
    
  virtual 
  dense_vector_t<T>& data(void* /*cont*/, const dense_vector_t<T>* /*tag*/) const 
  { 
    errTypeMismatch(type_name(), to_string(tag<T>())); 
  }
  
  virtual 
  sparse_vector_t<T>& data(void* /*cont*/, const sparse_vector_t<T>* /*tag*/) const 
  { 
    errTypeMismatch(type_name(), to_string(tag<T>()));  
  }
  
  virtual 
  T* at(void* /*cont*/, size_t /*pos*/, const T* /*tag*/) const 
  { 
    errTypeMismatch(type_name(), to_string(tag<T>())); 
  }
  
  virtual 
  const T& default_value(void* /*cont*/, const T* /*tag*/) const 
  { 
    errTypeMismatch(type_name(), to_string(tag<T>())); 
  }
  
  virtual 
  T& cell(void* /*cont*/, size_t /*pos*/, const T* /*tag*/) const 
  { 
    errTypeMismatch(type_name(), to_string(tag<T>())); 
  }
    
  virtual 
  void add(void* /*cont*/, T&&) const 
  { 
    errTypeMismatch(type_name(), to_string(tag<T>())); 
  }
    
  virtual 
  std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> > 
  range(void* /*cont*/, const T*) const 
  { 
    errTypeMismatch(type_name(), typeid(T).name()); 
  }  
};


// define the column types
struct VectorAccessorBase : VectorAccessorBaseT<string_t>
                          , VectorAccessorBaseT<int_t>
                          , VectorAccessorBaseT<uint_t> 
                          , VectorAccessorBaseT<real_t>
{
  /// returns true iff this is a sparse column
  virtual bool is_sparse() const = 0;
  
  virtual std::string type_name() const override = 0;

  virtual dataframe_variant_t cell_variant(void* /*cont*/, size_t /*pos*/) const = 0; 
  
  virtual void add_variant(void* /*cont*/, dataframe_variant_t&& /*elem*/) const = 0; 
  // may not be needed
  //~ virtual void cell_variant(void* /*cont*/, size_t /*pos*/, const dataframe_variant_t&) const = 0;

  /// writes back any data held in volatile memory
  virtual void persist(void* /*cont*/) const = 0;
};


template <template <class> class VectorT, class T>
struct VectorAccessorCommon : VectorAccessorBase
{
  using VectorRep = VectorT<T>; 

  VectorRep& 
  data(void* vec, const VectorRep* /*tag*/) const override
  {
    VectorRep* container = static_cast<VectorRep*>(vec);
    
    assert(container);
    return *container; 
  }
    
  bool is_sparse() const override 
  { 
    return std::is_same<VectorRep, sparse_vector_t<T> >::value;
  }
  
  std::string type_name() const override
  {
    return to_string(tag<T>());
  }  
  
  VectorRep& 
  data(void* vec) const 
  {
    return data(vec, tag<VectorRep>()); 
  }
  
  const T& 
  default_value(void* vec, const T* /*tag*/) const override
  { 
    return data(vec).default_value();
  } 
  
  dataframe_variant_t 
  cell_variant(void* vec, size_t row) const override
  {
    const VectorAccessorBaseT<T>& self = *this;
    T*                            res = self.at(vec, row, tag<T>());
    
    return res ? dataframe_variant_t{*res} : dataframe_variant_t{self.default_value(vec, tag<T>())};
  }
  
  void 
  add_variant(void* vec, dataframe_variant_t&& elem) const override
  {
    const VectorAccessorBaseT<T>& self = *this;
    
    self.add(vec, std::get<T>(std::move(elem)));
  }

/*  
  void cell_variant(void* vec, size_t pos, const dataframe_variant_t& el) const override
  { 
    const VectorAccessorBaseT<T>& self = *this;
    
    self.cell(vec, pos, tag<T>()) = std::get<T>(el); 
  }
*/
};

// dense vector accessor
template <class T>
struct DenseVectorAccessor : VectorAccessorCommon<dense_vector_t, T>
{ 
  using base = VectorAccessorCommon<dense_vector_t, T>;
  using typename base::VectorRep;
    
  T* 
  at(void* vec, size_t pos, const T*) const override
  {
    return &base::data(vec).at(pos);
  }
  
  T& 
  cell(void* vec, size_t pos, const T*) const override
  {
    return base::data(vec).at(pos);
  }

  void 
  add(void* vec, T&& el) const override
  {
    VectorRep& cont = base::data(vec);
    
    cont.emplace_back(std::move(el));
  }
  
  std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> > 
  range(void* cont, const T*) const override
  {
    using CommonIterator = AnyColumnIterator<T>;
    
    VectorRep& col = base::data(cont);
    
    return std::make_pair( CommonIterator{col.begin(), 0}, 
                           CommonIterator{col.end(), col.size()}
                         ); 
  }  
  
  void persist(void*) const override { /* nothing to do for dense vectors */ }
};

// sparse vector accessor
template <class T>
struct SparseVectorAccessor : VectorAccessorCommon<sparse_vector_t, T>
{ 
  using base = VectorAccessorCommon<sparse_vector_t, T>;
  using typename base::VectorRep; 
  
  T*
  at(void* vec, size_t row, const T*) const override
  {
    using VecIterator = typename VectorRep::iterator;
    
    VectorRep&  cont = base::data(vec);
    VecIterator pos  = cont.find(row);
    
    if (pos == cont.find_end()) 
    {
      // [[unlikely]] 
      return nullptr;
    }
    
    return &pos->second;
  }
  
  T&
  cell(void* vec, size_t row, const T*) const override
  {
    return base::data(vec)[row];
  }
  
  void 
  add(void* vec, T&& el) const override
  {
    VectorRep& cont = base::data(vec);
    
    cont.emplace(cont.size(), std::move(el));
  }

  std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> > 
  range(void* cont, const T*) const override
  {
    using CommonIterator = AnyColumnIterator<T>;
    
    VectorRep& col = base::data(cont);
    
    return std::make_pair( CommonIterator{col.begin()}, 
                           CommonIterator{col.end()}
                         ); 
  }
  
  void persist(void* cont) const override
  { 
    base::data(cont).persist(); 
  }  
};



struct VectorAccessorAny 
{    
    template <class T>
    explicit
    VectorAccessorAny(const dense_vector_t<T>* /* tag */)
    : v(new DenseVectorAccessor<T>)
    {}
    
    template <class T>
    explicit
    VectorAccessorAny(const sparse_vector_t<T>* /* tag */)
    : v(new SparseVectorAccessor<T>)
    {}
    
    ~VectorAccessorAny() = default;
    
    template <class T>
    T* at(void* cont, size_t row) const 
    {
      VectorAccessorBaseT<T>* obj = &*v;
      
      return obj->at(cont, row, tag<T>());
    }

    template <class T>
    const T& default_value(void* cont) const 
    {
      VectorAccessorBaseT<T>* obj = &*v;
      
      return obj->default_value(cont, tag<T>());
    }
    
    template <class T>
    T& cell(void* cont, size_t row) const 
    {
      VectorAccessorBaseT<T>* obj = &*v;
      
      return obj->cell(cont, row, tag<T>());
    }

    template <class T>
    void add(void* cont, T&& el) const 
    {
      VectorAccessorBaseT<T>* obj = &*v;
      
      assert(obj);
      obj->add(cont, std::move(el));
    }

    template <class ColType, class T = typename ColType::value_type>
    ColType& 
    data(void* cont) const 
    {
      VectorAccessorBaseT<T>* obj = &*v;
      
      return obj->data(cont, tag<ColType>()); 
    }
    
    template <class T>
    std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> > 
    range(void* cont) const 
    {
      VectorAccessorBaseT<T>* obj = &*v;
      
      return obj->range(cont, tag<T>());
    }
    
    dataframe_variant_t 
    cell_variant(void* vec, size_t pos) const
    {
      VectorAccessorBase* obj = &*v;
      
      return obj->cell_variant(vec, pos); 
    }
    
    void 
    add_variant(void* vec, dataframe_variant_t&& el) const
    {
      VectorAccessorBase* obj = &*v;
      
      return obj->add_variant(vec, std::move(el)); 
    }
    
    std::string 
    type_name() const
    {
      return v->type_name();
    }
    
    bool 
    is_sparse() const
    {
      return v->is_sparse();
    }
    
        
    void persist(void* cont) 
    {
      v->persist(cont);
    }
    
     
  private:  
    VectorAccessorAny()                                    = delete;
    VectorAccessorAny(const VectorAccessorAny&)            = delete;
    VectorAccessorAny(VectorAccessorAny&&)                 = delete;
    VectorAccessorAny& operator=(VectorAccessorAny&&)      = delete;
    VectorAccessorAny& operator=(const VectorAccessorAny&) = delete;
    
    const std::unique_ptr<VectorAccessorBase> v;
};


static VectorAccessorAny accessors[] = 
           {
             VectorAccessorAny{tag<DenseVectorAccessor<string_t>::VectorRep>()},
             VectorAccessorAny{tag<DenseVectorAccessor<int_t>::VectorRep>()},
             VectorAccessorAny{tag<DenseVectorAccessor<uint_t>::VectorRep>()},
             VectorAccessorAny{tag<DenseVectorAccessor<real_t>::VectorRep>()},
             VectorAccessorAny{tag<SparseVectorAccessor<string_t>::VectorRep>()},
             VectorAccessorAny{tag<SparseVectorAccessor<int_t>::VectorRep>()},
             VectorAccessorAny{tag<SparseVectorAccessor<uint_t>::VectorRep>()},
             VectorAccessorAny{tag<SparseVectorAccessor<real_t>::VectorRep>()},
           };


template <class ColType>
struct ColumnTraits 
{};

template <size_t kind, class T>
struct ColumnTraitsImpl 
{
  using type = T;
  
  enum { col = kind };
};


template <> 
struct ColumnTraits< dense_vector_t<string_t> > 
: ColumnTraitsImpl<0, string_t> {};

template <> 
struct ColumnTraits< dense_vector_t<int_t> >     
: ColumnTraitsImpl<1, int_t> {};

template <> 
struct ColumnTraits< dense_vector_t<uint_t> >    
: ColumnTraitsImpl<2, uint_t> {};

template <> 
struct ColumnTraits< dense_vector_t<real_t> >      
: ColumnTraitsImpl<3, real_t> {};

template <> 
struct ColumnTraits< sparse_vector_t<string_t> > 
: ColumnTraitsImpl<4, string_t> {};

template <> 
struct ColumnTraits< sparse_vector_t<int_t> >     
: ColumnTraitsImpl<5, int_t> {};

template <> 
struct ColumnTraits< sparse_vector_t<uint_t> >    
: ColumnTraitsImpl<6, uint_t> {};

template <> 
struct ColumnTraits< sparse_vector_t<real_t> >      
: ColumnTraitsImpl<7, real_t> {};

} // anonymous


template <class T>
struct cell_descriptor
{
    using type_name = T;
    
    cell_descriptor() : default_value() {}
    
    explicit
    cell_descriptor(const T& el) : default_value(el) {}
  
    explicit
    cell_descriptor(T&& el) : default_value(std::move(el)) {}
    
    cell_descriptor(const cell_descriptor&)            = default;
    cell_descriptor(cell_descriptor&&)                 = default;
    cell_descriptor& operator=(const cell_descriptor&) = default;
    cell_descriptor& operator=(cell_descriptor&&)      = default;
    ~cell_descriptor()                                 = default;
    
    T value() && { return std::move(default_value); } 
    
  private:
  
    T default_value;
};

template <class T>
struct sparse : cell_descriptor<T> 
{
  using base = cell_descriptor<T>;
  using base::base;
};

template <class T>
struct dense : cell_descriptor<T> 
{
  using base = cell_descriptor<T>;
  using base::base;
};

struct ColumnDesc
{
  std::string column_type;
  bool        is_sparse_column;
  
  template <class ColumnType>
  bool is() const { return column_type == to_string(tag<ColumnType>()); }
};

//~ template <class Key>
struct DataFrame
{  
    DataFrame(metall::create_only_t tag, const char* basepath, std::string dataframekey)
    : memmgr(tag, basepath), key(std::move(dataframekey))
    {
      allColumns  = memmgr.construct<dense_vector_t<ColumnOfsRep>>(key.c_str())(memmgr.get_allocator());
      allColNames = memmgr.construct<ColumnNames>((key + colnamesSuffix).c_str())(memmgr.get_allocator());
      numRows     = memmgr.construct<size_t>((key + numrowsSuffix).c_str())(0);
    }

    DataFrame(metall::open_only_t tag, const char* basepath, std::string dataframekey)
    : memmgr(tag, basepath), key(std::move(dataframekey))
    {
      allColumns  = memmgr.find<dense_vector_t<ColumnOfsRep>>(key.c_str()).first;      
      allColNames = memmgr.find<ColumnNames>((key + colnamesSuffix).c_str()).first;
      numRows     = memmgr.find<size_t>((key + numrowsSuffix).c_str()).first;
    }
    
    ~DataFrame() 
    {
      persist();
    }
  
    size_t rows() const
    {
      return *numRows;
    }
    
    size_t columns() const
    {
      return allColumns->size();
    }
  
    template <class... RowType>
    std::tuple<RowType...> 
    get_row(int row, const std::tuple<RowType...>*) const
    {
      return get_row<RowType...>(row, std::make_index_sequence<sizeof... (RowType)>()); 
    }
    
    template <class... RowType>
    std::tuple<RowType...> 
    get_row(int row, const std::tuple<RowType...>*, const std::vector<int>& idxlst) const 
    {
      return get_row_idxlst<RowType...>(row, idxlst, std::make_index_sequence<sizeof... (RowType)>()); 
    }
    
    
    std::vector<dataframe_variant_t> 
    get_row_variant(int row, const std::vector<int>& idxlst) const
    {
      std::vector<dataframe_variant_t> res;
      
      for (auto col : idxlst)
        res.emplace_back(get_cell_variant(row, col));  
      
      return res;
    }    
    

    std::vector<int>
    get_index_list(const std::vector<string_t>& colnames)
    {
      std::vector<int> res;
      
      for (const string_t& colname : colnames)
        res.push_back(colIdx(colname));
        
      return res;
    }

    template <class ColType>
    void set_cell(int row, int col, ColType&& el)
    {
      ColumnRep rep = colAt(col);
      
      accessors[rep.first].cell<ColType>(rep.second, row) = std::move(el);
    }

    // adds a new row
    template <class... RowType>
    void add(std::tuple<RowType...>&& el)
    {
      ++*numRows;
      return add_row<RowType...>(std::move(el), std::make_index_sequence<sizeof... (RowType)>());
    }
        
    void add_variant(std::vector<dataframe_variant_t>&& row)
    {
      std::size_t col = 0;
      
      ++*numRows;
      
      for (dataframe_variant_t& cell : row)
        add_col_variant(std::move(cell), col++);
    }
    
    //
    // add new columns
    

    /// add a single columns
    /// \{     
    template <class Column>
    void add_column_default_value(Column&& defaultval)
    {
      add_dense_column(defaultval);
    }
    
    template <class Column>
    void add_column_default_value(dense<Column>&& defval_wrapper)
    {
      add_dense_column(std::move(defval_wrapper).value());
    }
    
    template <class Column>
    void add_column_default_value(sparse<Column>&& defval_wrapper)
    {
      add_sparse_column(std::move(defval_wrapper).value());
    }
    
    void name_column(size_t i, const string_t& name)
    {
      assert(allColumns && i < allColumns->size());
      
      (*allColNames)[name] = i;
    }
    
    void name_column_std(size_t i, const std::string& name)
    {
      name_column(i, string_t{name.c_str()});
    }
    
    void name_last_column(const string_t& name)
    {
      assert(allColumns);
      
      const size_t idx = allColumns->size()-1;
      
      name_column(idx, name); 
    }

    void name_last_column_std(const std::string& name)
    {
      name_last_column(string_t{name.c_str()}); 
    }
    
    void name_columns(const std::vector<string_t>& names)
    {
      name_columns_internal(names, &DataFrame::name_column);
    }

    void name_columns_std(const std::vector<std::string>& names)
    {
      name_columns_internal(names, &DataFrame::name_column_std);
    }

    /// \}
    
    /// add multiple columns
    /// \{
    template <class... Columns, size_t... I>
    void add_columns_default_value(std::tuple<Columns...>&& cols, std::index_sequence<I...>)
    {
      (add_column_default_value(std::get<I>(cols)), ...);
    }
  
    template <class... Columns>
    void add_columns_default_value(std::tuple<Columns...>&& cols)
    {
      add_columns_default_value(std::move(cols), std::make_index_sequence<sizeof... (Columns)>());
    }
    /// \}
    
    
    /// add columns without default value
    /// \note the preferred method is to use add_columns_default_value
    /// \deprecated
    /// \{
    
    template <class... Columns>
    void add_columns()
    {
      (add_column(tag<Columns>()), ...);
    }

    template <class... Columns>
    void add_columns(const std::tuple<Columns...>*)
    {
      add_columns<Columns...>();
    }

    /// \}

    template <class T>
    std::pair<typename dense_vector_t<T>::iterator, typename dense_vector_t<T>::iterator>
    get_dense_column(size_t col) const
    {
      ColumnRep          rep = colAt(col);
      dense_vector_t<T>& vec = accessors[rep.first].data<dense_vector_t<T> >(rep.second);      
      
      return std::make_pair(vec.begin(), vec.end());
    }
    
    template <class T>
    std::pair<typename dense_vector_t<T>::iterator, typename dense_vector_t<T>::iterator>
    get_dense_column(const string_t& colname) const
    {
      return get_dense_column<T>(colIdx(colname)); 
    }
    
    template <class T>
    std::pair<typename dense_vector_t<T>::iterator, typename dense_vector_t<T>::iterator>
    get_dense_column_std(const std::string& colname) const
    {
      return get_dense_column<T>(string_t{colname.c_str()}); 
    }
    
    template <class T>
    std::pair<typename sparse_vector_t<T>::iterator, typename sparse_vector_t<T>::iterator>
    get_sparse_column(size_t col) const
    {
      ColumnRep           rep = colAt(col);
      sparse_vector_t<T>& vec = accessors[rep.first].data<sparse_vector_t<T>, T>(rep.second);      
      
      return std::make_pair(vec.begin(), vec.end());
    }

    template <class T>
    std::pair<typename sparse_vector_t<T>::iterator, typename sparse_vector_t<T>::iterator>
    get_sparse_column(const string_t& colname) const
    {
      return get_sparse_column<T>(colIdx(colname));
    }

    template <class T>
    std::pair<typename sparse_vector_t<T>::iterator, typename sparse_vector_t<T>::iterator>
    get_sparse_column_std(const std::string& colname) const
    {
      return get_sparse_column<T>(string_t{colname.c_str()});
    }

    template <class T>
    std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> >
    get_any_column(size_t col) const
    {
      ColumnRep rep = colAt(col);
      
      return accessors[rep.first].range<T>(rep.second);      
    }
    
    template <class T>
    std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> >
    get_any_column(const string_t& colname) const
    {
      return get_sparse_column<T>(colIdx(colname));      
    }

    template <class T>
    std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> >
    get_any_column_std(const std::string& colname) const
    {
      return get_sparse_column<T>(string_t{colname.c_str()});      
    }
    
    std::vector<ColumnDesc>
    get_column_descriptors(const std::vector<int>& idxlst) const
    {
      std::vector<ColumnDesc> res;
      
      for (int idx : idxlst)
        res.push_back(get_column_descriptor(idx));
      
      return res;  
    }
    
    std::vector<std::string>
    get_column_names() const
    {
      assert(allColNames);
      
      std::vector<std::string> res{columns(), std::string{}};
      
      for (auto& colname : (*allColNames))
        res.at(colname.second) = std::string{colname.first.c_str()};
      
      return res;
    }
    
    
    std::vector<ColumnDesc>
    get_column_descriptors(const std::vector<string_t>& colnames)
    {
      return get_column_descriptors(get_index_list(colnames));
    }
    
    void persist() const
    {
      // persist all columns
      for (size_t max = allColumns->size(), col = 0; col < max; ++col)
      {
        ColumnRep rep = colAt(col);
        
        accessors[rep.first].persist(rep.second);
      }
      
      // persist column names
      allColNames->persist();
    }
    
    dataframe_variant_t 
    get_cell_variant(int row, int col) const
    {
      ColumnRep rep = colAt(col);
      
      return accessors[rep.first].cell_variant(rep.second, row);
    }

    
  
/*  
    template <class... RowType>
    void xchg(int row, std::tuple<RowType...>&);
  
    template <class ColType>
    void xchg(int row, int col, ColType&);
*/
        
  private:
    //
    // types
    
    using VoidOfsPtr   = metall::offset_ptr<void>;
    using ColumnOfsRep = std::pair<int, VoidOfsPtr>;
    using ColumnRep    = std::pair<int, void*>;
    using ColumnNames  = flat_map<string_t, size_t>;
    
    //
    // data

    // not in persistent memory
    metall::manager               memmgr;
    std::string                   key;
    
    // in persistent memory
    dense_vector_t<ColumnOfsRep>* allColumns  = nullptr; ///< stores all columns
    ColumnNames*                  allColNames = nullptr; ///< stores all column names 
    size_t*                       numRows     = nullptr; ///< quick access to number of rows 
                                                         ///  (no need to query from columns)
                                               
    //
    // constants
    static constexpr const char* const colnamesSuffix = "~names";                                             
    static constexpr const char* const numrowsSuffix  = "~size";                                             
    

    // 
    // internal functions
    
    /// returns a descriptor (kind, void pointer to container) for a
    ///   specified column.
    /// \private
    ColumnRep 
    colAt(size_t col) const
    { 
      const ColumnOfsRep& desc = allColumns->at(col);
      
      return ColumnRep{desc.first, metall::to_raw_pointer(desc.second)}; 
    }
    
    size_t colIdx(const string_t& name) const
    {
      using iterator = typename ColumnNames::iterator;
      
      assert(allColNames);
      
      iterator pos = allColNames->find(name);
      
      if (pos == allColNames->find_end())
      {
        // [[unlikely]]
        throw unknown_column_error(name + " is not a known column");
      }
      
      return pos->second;
    }
        
    template <class... ColType, size_t... I>
    std::tuple<ColType...> 
    get_row(int row, std::index_sequence<I...>) const
    {
      return std::tuple<ColType...>{ get_cell(row, I, tag<ColType>())... };
    }
    
    template <class... ColType, size_t... I>
    std::tuple<ColType...> 
    get_row_idxlst(int row, const std::vector<int>& idxlst, std::index_sequence<I...>) const
    {
      return std::tuple<ColType...>{ get_cell(row, idxlst.at(I), tag<ColType>())... };
    }

    template <class ColType>
    void 
    add_col_val(ColType&& el, size_t col)
    {
      ColumnRep rep = colAt(col);
      
      accessors[rep.first].add<ColType>(rep.second, std::move(el));
    }
    
    void 
    add_col_variant(dataframe_variant_t&& el, size_t col)
    {
      ColumnRep rep = colAt(col);
      
      accessors[rep.first].add_variant(rep.second, std::move(el));
    }
    
    template <class... RowType, size_t... I>
    void 
    add_row(std::tuple<RowType...>&& el, std::index_sequence<I...>)
    {
      (add_col_val<RowType>(std::move(std::get<I>(el)), I), ...);
    }
    
    template <class Column>
    void add_column(Column*)
    {
      add_dense_column(Column{});
    }
    
    template <class Column>
    void add_column(const dense<Column>*)
    {
      add_dense_column(Column{});
    }
    
    template <class Column>
    void add_column(const sparse<Column>*)
    {
      add_sparse_column(Column{});
    }
        
    template <class T>
    void add_dense_column(const T& defaultval)
    {
      std::string colkey{key};
      
      colkey.append('~', 1);
      colkey.append(boost::lexical_cast<std::string>(allColumns->size())); 
      
      dense_vector_t<T>* newcol = memmgr.construct<dense_vector_t<T> >(colkey.c_str())(memmgr.get_allocator());
      
      assert(newcol);
      newcol->resize(rows(), defaultval);
      newcol->default_value(T{defaultval});
      allColumns->emplace_back(ColumnTraits<dense_vector_t<T> >::col, newcol);
    }
        
    template <class T>
    void add_sparse_column(T&& defaultval)
    {
      std::string colkey{key};
      
      colkey.append('~', 1);
      colkey.append(boost::lexical_cast<std::string>(allColumns->size())); 
      
      sparse_vector_t<T>* newcol = memmgr.construct<sparse_vector_t<T> >(colkey.c_str())(memmgr.get_allocator());
      //~ sparse_vector_t<T>* newcol = new sparse_vector_t<T>;
      
      assert(newcol);
      newcol->default_value(std::move(defaultval));
      allColumns->emplace_back(ColumnTraits<sparse_vector_t<T> >::col, newcol);
    }

    template <class ColType>
    const ColType&
    get_cell(size_t row, size_t col, const ColType*) const
    {
      ColumnRep rep  = colAt(col);
      ColType*  elem = accessors[rep.first].at<ColType>(rep.second, row);
      
      if (!elem) 
      { 
        // [[unlikely]]
        return accessors[rep.first].default_value<ColType>(rep.second);
        //~ throw std::logic_error("cell value not available");
      }
      
      return *elem;
    }

/*    
    template <class ColType>
    std::optional<ColType>
    get_cell(size_t row, size_t col, const std::optional<ColType>*) const
    {
      ColumnRep rep = colAt(col);
      ColType*  elem = accessors[rep.first].at<ColType>(rep.second, row);
      
      if (!elem)
      {
        // [[unlikely]]
        return std::optional<ColType>{};
      }  
      
      return *elem;
    }
*/
    
    ColumnDesc
    get_column_descriptor(int col) const
    {
      ColumnRep          rep = colAt(col);
      VectorAccessorAny& column = accessors[rep.first];
      
      return ColumnDesc{column.type_name(), column.is_sparse()};
    }
    
    template <class StringT>
    void name_columns_internal( const std::vector<StringT>& names, 
                                void (DataFrame::*fn) (size_t, const StringT& el)
                              )
    {
      for (size_t max = names.size(), i = 0; i < max; ++i)
        (this->*fn)(i, names[i]);
    }
};

template <class Fn, class T>
void forCell( Fn& fn, 
              size_t row, size_t col,
              std::pair<AnyColumnIterator<T>, AnyColumnIterator<T> >& range
            )
{
  assert((range.first == range.second) || (range.first.row() >= row));
  
  if (!(range.first == range.second) && range.first.row() == row)
  {
    fn(row, col, std::optional<T>{*range.first});
    ++range.first;
  }
  else
  {
    fn(row, col, std::optional<T>{});
  }
}

template <class Fn, class T>
void forCell( Fn& fn, 
              size_t row, size_t col,
              std::pair< typename sparse_vector_t<T>::iterator, 
                         typename sparse_vector_t<T>::iterator
                       >& range
            )
{
  assert((range.first == range.second) || (range.first->first >= row));
  
  if (!(range.first == range.second) && range.first->first == row)
  {
    fn(row, col, std::optional<T>{range.first->second});
    ++range.first;
  }
  else
  {
    fn(row, col, std::optional<T>{});
  }
}


template <class Fn, class T>
void forCell( Fn& fn, 
              size_t row, size_t col,
              std::pair< typename dense_vector_t<T>::iterator, 
                         typename dense_vector_t<T>::iterator
                       >& range
            )
{
  assert(range.first != range.second);
  
  fn(row, col, std::optional<T>{*range.first});
  ++range.first;
}

//~ template <template <class, class> class Pair>
template <class T>
struct MappedType
{
  using type = T;
};

template <class T>
struct MappedType<std::pair<const size_t, T> >
{
  using type = T;
};

template <class Pair>
struct ColumnValueType {}; 


template <class Iter>
struct ColumnValueType<std::pair<Iter, Iter> >
{
  using type = typename MappedType<typename std::iterator_traits<Iter>::value_type>::type;
};


template <class Fn, class... ColumnRange>
Fn forallRows(Fn fn, size_t rowlimit, ColumnRange... columns)
{
  for (size_t row = 0; row < rowlimit; ++row)
  {
    size_t col = 0;
    
    (forCell<Fn, typename ColumnValueType<ColumnRange>::type>(fn, row, col++, columns), ...);
  }  
  
  return fn;
} 


} // namespace experimental


