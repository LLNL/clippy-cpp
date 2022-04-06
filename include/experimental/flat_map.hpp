#pragma once

//~ #include <vector>
#include <utility>
#include <functional>
#include <exception>
#include <algorithm>
#include <memory>
#include <cassert>

#include <boost/container/vector.hpp>

#include "metall/metall.hpp"
#include "metall/tags.hpp"

// use parallel algorithms only if specifically requested
//   helps avoid issues with missing TBB which is required for GCC parallel algorithms
#ifdef WITH_PARALLEL_STL

// portability for pre-C++20 compilers
#if defined __has_include
  #if __has_include (<execution>)
  #define CXX20_PARALLEL_EXECUTION 1
	#include <execution>
  #endif
#endif /* defined __has_include */

#endif /* WITH_PARALLEL_STL */

#ifdef CXX20_PARALLEL_EXECUTION
#define EXEC_STRATEGY PAR_STRATEGY,
#else
#define EXEC_STRATEGY
#endif /* CXX20_PARALLEL_EXECUTION */

namespace experimental
{

namespace
{
  constexpr bool HOMEGROWN_ALGORITHMS           = true;
  constexpr int  LINEAR_BINARY_SEARCH_THRESHOLD = (1<<7);
  constexpr int  MERGE_ALGORITHM_VERSION        = 4;
  // 0 .. add + sort(all)
  // 1 .. sort(new) + add + in_place_merge
  // 2 .. sort(new) + merge
  // 3 .. sort(new) + lower_bound + move + insert
  // 4 .. sort(new) + move_until(linear) + insert (good for int keys, not good for strings)
  
#ifdef CXX20_PARALLEL_EXECUTION
  constexpr auto PAR_STRATEGY                   = std::execution::seq;  
  //~ constexpr auto PAR_STRATEGY                   = std::execution::unseq;  
#endif /* CXX20_PARALLEL_EXECUTION */

  template <class KeyComp>
  struct KeyValComparator
  {
    explicit
    KeyValComparator(KeyComp cmp)
    : comp(cmp)
    {}

    template <class KeyT, class ElemT>
    bool operator()(const std::pair<KeyT, ElemT>& val, const KeyT& key)
    {
      return comp(val.first, key);
    }

    KeyComp     comp;
  };

  template <class ForwardIterator, class T, class Comparator>
  ForwardIterator
  lowerBoundLinear(ForwardIterator first, const ForwardIterator limit, const T& el, Comparator comp)
  {
    while (first != limit && comp(*first, el))
    {
      ++first;
    }

    return first;
  }

  template <class RandomAccessIterator, class T, class Comparator>
  RandomAccessIterator
  lowerBound(RandomAccessIterator first, RandomAccessIterator limit, const T& el, Comparator comp)
  {
    if constexpr (!HOMEGROWN_ALGORITHMS)
      return std::lower_bound(first, limit, el, comp);
      
    std::size_t dist = std::distance(first, limit);

    while (dist >= LINEAR_BINARY_SEARCH_THRESHOLD)
    {
      RandomAccessIterator cand = first + (dist/2);
/*
      const int            less = comp(*cand, el);

      first = cand + less;
			limit = limit - std::distance(cand, limit) * (1-less);
*/
      if (comp(*cand, el))
        first = cand+1;
      else
        limit = cand;
        
      dist = std::distance(first, limit);
    }

    return lowerBoundLinear(first, limit, el, comp);
  }
  
  template <class ForwardIterator, class T, class Comparator>
  ForwardIterator
  upperBoundLinear(ForwardIterator first, const ForwardIterator limit, const T& el, Comparator comp)
  {
    while (first != limit && !comp(el, *first))
    {
      ++first;
    }

    return first;
  }
  
  template <class RandomAccessIterator, class T, class Comparator>
  RandomAccessIterator
  upperBound(RandomAccessIterator first, RandomAccessIterator limit, const T& el, Comparator comp)
  {
    if constexpr (!HOMEGROWN_ALGORITHMS)
      return std::lower_bound(first, limit, el, comp);      

    std::size_t dist = std::distance(first, limit);
    
    //~ __builtin_prefetch(&first[dist/2 + dist/4]);
    //~ __builtin_prefetch(&first[dist/2 - dist/4]);
    
    while (dist >= LINEAR_BINARY_SEARCH_THRESHOLD)
    {
      RandomAccessIterator cand = first + (dist/2);
      
      if (!comp(el, *cand))
        first = cand+1;
      else
        limit = cand;
        
      dist = std::distance(first, limit);
      //~ __builtin_prefetch(&first[dist/2 + dist/4]);
      //~ __builtin_prefetch(&first[dist/2 - dist/4]);
    }
    
    return upperBoundLinear(first, limit, el, comp);
  }
}


/// A flat map implementation that stores the data in a contiguous vector. The
///   interface is reminiscent of bit not the same as STL's sorted associative containers. 
///   Major differences compare to the standard specification are:
///   - only a subset of operations are implemented
///   - some operations invoke persist implicitly (which reorganizes the data structure internally).
///   - to test whether an element is in the data structure one calls
///     \ref find and compare the returned result with \ref find_end instead
///     of \ref end.
/// \details
///   To attain efficient insert operations, the inserted elements are buffered in
///   an unspecified auxiliary data structure, until \ref persist is called. 
///   Some operations call persist internally. The buffer uses an STL allocator.
/// \todo 
///   Similar to insert, delete and other structural update operations could be
///   buffered (not yet implemented).
/// \tparam KeyT    the key type
/// \tparam ElemT   the element (mapped) type
/// \tparam Compare the key comparator 
/// \tparam Alloc   the allocator used for the flat storage (BUT NOT for the 
///                 temporary buffer).
/// \tparam Vector  the implementation of the flat data structure. It will be
///                 parameterized as Vector<std::pair<KeyT, ElemT>, Alloc>.
template < class KeyT,
           class ElemT,
           class Compare = std::less<KeyT>,
           class Alloc = metall::manager::allocator_type<std::pair<KeyT, ElemT> >,
           template <class, class> class Vector = boost::container::vector
         >
struct flat_map : /* private */ Vector<std::pair<KeyT, ElemT>, Alloc>
{
  // external types
    using key_compare    = Compare;
    using allocator_type = Alloc;
    using value_type     = std::pair<const KeyT, ElemT>;
    using key_type       = KeyT;
    using mapped_type    = ElemT;

    using size_type      = std::size_t;
    using pointer        = value_type*;
    using reference      = value_type&;
    using iterator       = value_type*;
    using const_iterator = const value_type*;

  private:
    using base               = Vector<std::pair<KeyT, ElemT>, Alloc>;
    // internal types
    using value_type_vec     = typename base::value_type;
    using pointer_vec        = value_type_vec*;
    using reference_vec      = value_type_vec&;
    using iterator_vec       = value_type_vec*;
    using const_iterator_vec = const value_type_vec*;
    using buffer_type        = std::map<key_type, mapped_type, key_compare>;

  public:
  // constructors
    flat_map() = delete;

    /// creates an empty flat_map
    /// \param compFn    the comparator
    /// \param alloc     the allocator
    /// \param prototype a prototype object that is used as the default object
    ///                  for internal resizing operations. The prototype object
    ///                  is required if the default constructor cannot be called,
    ///                  e.g., when an allocator needs to be passed to key or value. 
    flat_map( const key_compare& compFn, 
              const allocator_type& alloc = allocator_type(), 
              value_type prototype = value_type{}
            )
    : base(alloc), keycomp(compFn), sample(std::move(prototype))
    {}

    /// \param alloc     the allocator
    /// \param prototype a prototype object that is used as the default object
    ///                  for internal resizing operations.
    flat_map(const allocator_type& alloc, value_type prototype = value_type{})
    : base(alloc), keycomp(), sample(std::move(prototype))
    {}
    
    // \todo add cctor, mctor and ops=

  // iterator access
    /// returns an iterator to the first element in the flat map (invokes persist)
    iterator begin()
    {
      persist();

      return reinterpret_cast<iterator>(vecbegin());
    }

    /// returns a const iterator to the first element in the flat map (invokes persist)
    const_iterator begin() const
    {
      persist();

      return vecbegin();
    }

    /// returns an iterator to the limit of the flat map (invokes persist)
    iterator end()
    {
      return begin() + base::size();
    }

    /// returns a const iterator to the limit of the flat map (invokes persist)
    const_iterator end() const
    {
      return begin() + base::size();
    }


  // capacity functions
    /// returns the maximum size of the this data structure
    std::size_t max_size() const noexcept
    {
      return base::max_size();
    }

    /// returns true iff the data structure is empty, false otherwise
    bool        empty()    const { return size() == 0; }
    
    /// returns the number of elements in the data structure
    std::size_t size()     const { return size<>(buf(this)); }
    
  private:
    /// returns the total number of elements, base::size() + buf.size()
    template <class BufferT>
    std::size_t size(const BufferT& buf) const 
    { 
      return base::size() + buf.size(); 
    }
    
  public:

  // element access
    /// returns a reference to the mapped value of \ref k (does NOT call persist).
    /// inserts a new element if the key is not available.
    mapped_type& operator[] (const key_type& k)
    {
      iterator pos = find(k);

      if (pos != find_end())
        return pos->second;

      return insert_buf(k, sample.second).second;
    }

    /// returns a reference to the mapped value of \ref k (does NOT call persist).
    /// inserts a new element if the key is not available.
    mapped_type& operator[] (key_type&& k)
    {
      iterator pos = find(k);

      if (pos != find_end()) return pos->second;

      return insert_buf(k, sample.second).second;
    }

    /// returns a reference to the mapped value of \ref k (does NOT call persist).
    /// throws an out_of_range if the key \ref k is not found.
    mapped_type& at(const key_type& k)
    {
      iterator pos = find(k);

      if (pos == find_end())
        throw std::out_of_range{"no element with key"};

      return *pos;
    }

    /// returns a const reference to the mapped value of \ref k (does NOT call persist).
    /// throws an out_of_range if the key \ref k is not found.
    const mapped_type& at(const key_type& k) const
    {
      const_iterator pos = find(k);

      if (pos == find_end())
        throw std::out_of_range{"no element with key"};

      return *pos;
    }

  // modifiers
    /// inserts a new element \ref val
    std::pair<iterator, bool>
    insert(const value_type& val)
    {
      pointer pos = find(val.first);

      if (pos != find_end())
        return std::make_pair(pos, false);

      return std::make_pair(insert_buf(val.first, val.second), true);
    }

    /// inserts a new element \ref val using move semantics
    template <class P>
    std::pair<iterator,bool>
    insert(P&& val)
    {
      const pointer pos = find(val.first);

      if (pos != find_end())
        return std::make_pair(pos, false);

      return std::make_pair(&insert_buf(std::move(val)), true);
    }
    
    /// emplaces a new element constructed from \ref args
    template <class... Args>
    std::pair<iterator,bool> emplace (Args&&... args)
    {
      value_type val{std::forward<Args>(args)...};
      
      return insert(std::move(val));
    }

    /// erases the element with \ref key k
    /// \details if the element is in the flat storage, the storage is
    ///          updated in an O(n) operation.
    /// \todo buffer erase operations.
    size_type
    erase(const key_type& k)
    {
      // \todo possibly pass buffer into find_internal
      std::pair<pointer_vec, bool> pos = find_internal(k);

      if (pos == find_end()) return 0;

      if (pos.second)
      {
        assert(pos >= vecbegin() && pos < vecend());

        base::erase(base::begin() + std::distance(vecbegin(), pos));
      }
      else
      {
        const bool deleted = buf(this).erase(k);
        
        assert(deleted);
      }

      return 1;
    }


/*
    iterator erase (const_iterator position);

    iterator insert (const_iterator position, const value_type& val);
    template <class P> iterator insert (const_iterator position, P&& val);

    template <class InputIterator>
    void insert (InputIterator first, InputIterator last);

    void insert (initializer_list<value_type> il);

    iterator erase (const_iterator first, const_iterator last);

    template <class... Args>
    iterator emplace_hint (const_iterator position, Args&&... args);
*/

    /// swaps this with that
    void swap (flat_map& that)
    {
      base::swap(that.container);
      
      buf(this).swap(buf(&that));
      // swap keycomp??
    }

    /// clears the container
    void clear() noexcept
    {
      base::clear();
      buf(this).clear();
    }

  // operations
    /// returns the limit of the find operation
    const_iterator find_end() const
    {
      return reinterpret_cast<const_iterator>(vecend());
    }

    /// returns the limit of the find operation
    iterator find_end()
    {
      return reinterpret_cast<iterator>(vecend());
    }
    
    /// returns an iterator to the elemnt with key \ref k.
    /// if the element is not found the value of find_end() is returned.
    const_iterator 
    find(const key_type& k) const
    {
      return reinterpret_cast<const_iterator>(find_internal(k).first);
    }

    /// returns an iterator to the elemnt with key \ref k.
    /// if the element is not found the value of find_end() is returned.
    iterator find(const key_type& k)
    {
      return nonConstIterator(constSelf().find(k));
    }

    /// returns 1 if an element with key \k exists, 0 otherwise
    size_type count(const key_type& k) const
    {
      return int(find(k) != find_end());
    }

    /// returns the lower bound of key \ref k (implies persist).
    const_iterator lower_bound(const key_type& k) const
    {
      return lowerBound(begin(), end(), k, comparator());
    }

    /// returns the lower bound of key \ref k (implies persist).
    iterator lower_bound(const key_type& k)
    {
      return lowerBound(begin(), end(), k, comparator());
    }

    /// returns the upper bound of key \ref k (implies persist).
    const_iterator upper_bound(const key_type& k) const
    {
      return upperBound(begin(), end(), k, comparator());
    }

    /// returns the upper bound of key \ref k (implies persist).
    iterator upper_bound(const key_type& k)
    {
      return upperBound(begin(), end(), k, comparator());
    }

    /// returns an iterator range for key \ref k (implies persist).
    std::pair<iterator,iterator>
    equal_range(const key_type& k)
    {
      return std::equal_range(begin(), end(), k, comparator());
    }

    /// returns an iterator range for key \ref k (implies persist).
    std::pair<const_iterator,const_iterator>
    equal_range(const key_type& k) const
    {
      return std::equal_range(begin(), end(), k, comparator());
    }


  // observers
    /// returns a copy of the comparator
    key_compare key_comp() const { return keycomp; }
    //~ using container::value_comp;

  // allocator
    /// returns the allocator object
    allocator_type
    get_allocator() const noexcept
    {
      return base::get_allocator();
    }

    /// integrates \ref buffer into the flat container
    /// \tparam BufferT a sorted container that supports 
    ///                 the following operations: size, clear, begin, end rbegin, rend 
    ///                 and the following type: reverse_iterator
    /// \param buffer   a sorted container from where the elements are moved into the
    ///                 the flat storage.
    /// \param sample   a default object used to resize the flat container 
    /// \details
    ///   the constants MERGE_ALGORITHM_VERSION, HOMEGROWN_ALGORITHMS,  
    ///   LINEAR_BINARY_SEARCH_THRESHOLD, and PAR_STRATEGY can be adjusted for
    ///   fine tuning.
    template <class BufferT>
    void integrate_sorted_buffer(BufferT& buffer)
    {
      using buf_reverse_iterator = typename buffer_type::reverse_iterator;
      
      size_t displacement = buffer.size();
      
      if (displacement == 0) return;

      key_compare compfn = key_comp();

      auto comp = [compfn](const value_type& lhs, const value_type& rhs) -> bool
                  {
                    return compfn(lhs.first, rhs.first);
                  };

      switch (MERGE_ALGORITHM_VERSION)
      {
          case 4:
          {
            base::resize(size(buffer), sample);

            const iterator_vec   start = vecbegin() - 1;
            iterator_vec         pos   = vecend() - displacement - 1;
            buf_reverse_iterator aa    = buffer.rbegin();
            buf_reverse_iterator zz    = buffer.rend();
            
            // O(N) key comparisons 
            while (aa != zz)
            {              
              while (pos != start && (comp(*aa, *pos)))
              {
                *(pos+displacement) = std::move(*pos);
                --pos;
              }

              *(pos+displacement) = *aa;
              --displacement;
              ++aa;
            }

            break;
          }
        case 3:
          {
            base::resize(size(buffer), sample);

            iterator_vec         limit = vecend() - displacement;
            buf_reverse_iterator aa    = buffer.rbegin();
            buf_reverse_iterator zz    = buffer.rend();
            

            // O(i * log(N)) key comparisons
            while (displacement > 0)
            {
              assert(aa != zz);
              
              iterator_vec pos = upperBound(vecbegin(), limit, *aa, comp);

              // \todo if in STL: std::shift_right(EXEC_STRATEGY pos, limit, i);
              std::move_backward(pos, limit, limit+displacement);

              --displacement;
              *(pos+displacement) = *aa;
              limit = pos;
              ++aa;
            }

            break;
          }

#if 0
        case 2:
        {
          base tmp; /* does currently not work w/ Metall */

          tmp.reserve(size(buffer));

          std::merge( EXEC_STRATEGY 
                      vecbegin(), vecend(), 
                      buffer.begin(), buffer.end(), 
                      std::back_inserter(tmp), 
                      comp
                    );
          base::swap(tmp);
          break;
        }
#endif
        case 1:
        {
          const size_t currsize = base::size();

          base::reserve(size(buffer));
          std::copy( EXEC_STRATEGY 
                     buffer.begin(), buffer.end(), 
                     std::back_inserter(*this)
                   );

          std::inplace_merge( EXEC_STRATEGY 
                              base::begin(), base::begin()+currsize, 
                              base::end(), 
                              comp
                            );
          break;
        }
        
        case 0:
        {
          base::reserve(size(buffer));
          std::copy( EXEC_STRATEGY 
                     buffer.begin(), buffer.end(), 
                     std::back_inserter(*this)
                   );
          std::sort(EXEC_STRATEGY base::begin(), base::end(), comp);
          break;
        }
        default: assert(false);
      }

      buffer.clear();
    }
    
  // additional public functions
    void persist()
    {
      integrate_sorted_buffer(buf(this));
    }

  private:

    // additional functions

    flat_map& nonConstSelf() const
    {
      return const_cast<flat_map&>(*this);
    }

    const flat_map& constSelf()
    {
      return *this;
    }
    
    std::pair<const_iterator, bool> 
    find_internal(const key_type& k) const
    {
      using buf_const_iterator = typename buffer_type::const_iterator;
      
      {
        const_iterator_vec const veclimit = vecend();
        const_iterator_vec       pos = lowerBound(vecbegin(), veclimit, k, comparator());

        if (pos != veclimit && (!keycomp(k, pos->first)))
          return std::make_pair(reinterpret_cast<const_iterator>(pos), true);
      }

      {
      /*  
        key_compare              compfn   = key_comp();
        const_iterator_vec const buflimit = buf + i;
        const_iterator_vec       pos = std::find_if( EXEC_STRATEGY
                                                 buf + 0, buflimit,
                                                 [compfn, k](const value_type& v) -> bool
                                                 {
                                                   return !compfn(v.first, k) && !compfn(k, v.first);
                                                 }
                                               );
      */
        buf_const_iterator buflimit = buf(this).end();
        buf_const_iterator bufpos   = buf(this).find(k);
         
        if (bufpos != buflimit)
          return std::make_pair(&*bufpos, true);
      }

      return std::make_pair(find_end(), false);
    }

    void persist() const
    {
      nonConstSelf().persist();
    }

    iterator nonConstIterator(const_iterator it)
    {
      return &const_cast<value_type&>(*it);
    }

    const_iterator_vec vecbegin() const
    {
      return base::data();
    }

    iterator_vec vecbegin()
    {
      return base::data();
    }

    const_iterator_vec vecend() const
    {
      return vecbegin() + base::size();
    }

    iterator_vec vecend()
    {
      return vecbegin() + base::size();
    }

    KeyValComparator<key_compare>
    comparator() const
    {
      return KeyValComparator<key_compare>{keycomp};
    }

    reference insert_buf(const KeyT& k, const ElemT& el)
    {
      buffer_type& buffer = buf(this);
      
      //~ if (i == N) persist(buffer);
/*
      buf[i] = std::make_pair(k, el);
      return buf[i++];
*/
      return *(buffer.insert(std::make_pair(k, el)).first);
    }

    reference insert_buf(value_type&& v)
    {
      buffer_type& buffer = buf(this);
      
      //~ if (i == N) persist(buffer);

/*
      buf[i] = std::move(v);
      return buf[i++];
*/
      return *(buffer.insert(std::move(v)).first);
    }
    
    static
    buffer_type& buf(flat_map* /* mainMap */)
    {
      static buffer_type dummy;
      
      return dummy;
      //~ return buffers[mainMap];
    }
    
    static
    const buffer_type& buf(const flat_map* /*mainMap*/)
    {
      static buffer_type dummy;
      
      return dummy;
      //~ return buffers[mainMap];
    }

    // data members
    //~ container_type container;
    key_compare    keycomp;
    value_type     sample;
    //~ size_type      i;  // insert at
    //~ size_type      d;  // delete at (N-1-d)
    //~ value_type_vec buf[N];
    //~ buffer_type    buf;
    //~ std::map<key_type, mapped_type> buf[N];
    
    //~ static std::unordered_map<const flat_map*, buffer_type, key_compare> buffers;
};

/*
template < class KeyT,
           class ElemT,
           class Compare,
           class Alloc,
           template <class, class> class Vector
         >
std::unordered_map< const flat_map<KeyT, ElemT, Compare, Alloc, Vector>*, 
                    std::map<KeyT, ElemT, Compare> 
                  >
flat_map<KeyT, ElemT, Compare, Alloc, Vector>::buffers;
*/

} // namespace experimental



