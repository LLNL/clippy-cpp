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
    if (!HOMEGROWN_ALGORITHMS)
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
    if (!HOMEGROWN_ALGORITHMS)
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

    explicit
    flat_map(const key_compare& compFn, const allocator_type& alloc = allocator_type())
    : base(alloc), keycomp(compFn)
    {}

    explicit
    flat_map(const allocator_type& alloc)
    : base(alloc), keycomp()
    {}
    
    // \todo add cctor, mctor and ops=

  // iterator access
    iterator begin()
    {
      persist();

      return reinterpret_cast<iterator>(vecbegin());
    }

    const_iterator begin() const
    {
      persist();

      return vecbegin();
    }

    iterator end()
    {
      return begin() + base::size();
    }

    const_iterator end() const
    {
      return begin() + base::size();
    }


  // capacity functions
    std::size_t max_size() const noexcept
    {
      return base::max_size();
    }

    bool        empty()    const { return size() == 0; }
    std::size_t size()     const { return size(buf(this)); }
    
    std::size_t size(const buffer_type& buf) const 
    { 
      return base::size() + buf.size(); 
    }

  // element access
    mapped_type& operator[] (const key_type& k)
    {
      iterator pos = find(k);

      if (pos != find_end())
        return pos->second;

      return insert_buf(k, mapped_type{}).second;
    }

    mapped_type& operator[] (key_type&& k)
    {
      iterator pos = find(k);

      if (pos != find_end()) return pos->second;

      return insert_buf(k, mapped_type{}).second;
    }

    mapped_type& at(const key_type& k)
    {
      iterator pos = find(k);

      if (pos == find_end())
        throw std::out_of_range{"no element with key"};

      return *pos;
    }

    const mapped_type& at(const key_type& k) const
    {
      const_iterator pos = find(k);

      if (pos == find_end())
        throw std::out_of_range{"no element with key"};

      return *pos;
    }

  // modifiers
    std::pair<iterator, bool>
    insert(const value_type& val)
    {
      pointer pos = find(val.first);

      if (pos != find_end())
        return std::make_pair(pos, false);

      return std::make_pair(insert_buf(val.first, val.second), true);
    }

    template <class P>
    std::pair<iterator,bool>
    insert(P&& val)
    {
      const pointer pos = find(val.first);

      if (pos != find_end())
        return std::make_pair(pos, false);

      return std::make_pair(&insert_buf(std::move(val)), true);
    }
    
    template <class... Args>
    std::pair<iterator,bool> emplace (Args&&... args)
    {
      value_type val{std::forward<Args>(args)...};
      
      return insert(std::move(val));
    }


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

    void swap (flat_map& x)
    {
      base::swap(x.container);
      
      buf(this).swap(buf(&x));
      // swap keycomp??
    }

    void clear() noexcept
    {
      base::clear();
      buf(this).clear();
    }

  // operations
    const_iterator find_end() const
    {
      return reinterpret_cast<const_iterator>(vecend());
    }

    iterator find_end()
    {
      return reinterpret_cast<iterator>(vecend());
    }
    
    const_iterator 
    find(const key_type& k) const
    {
      return reinterpret_cast<const_iterator>(find_internal(k).first);
    }

    iterator find(const key_type& k)
    {
      return nonConstIterator(constSelf().find(k));
    }

    size_type count(const key_type& k) const
    {
      return int(find(k) != find_end());
    }

    const_iterator lower_bound(const key_type& k) const
    {
      return lowerBound(begin(), end(), k, comparator());
    }

    iterator lower_bound(const key_type& k)
    {
      return lowerBound(begin(), end(), k, comparator());
    }

    const_iterator upper_bound(const key_type& k) const
    {
      return upperBound(begin(), end(), k, comparator());
    }

    iterator upper_bound(const key_type& k)
    {
      return upperBound(begin(), end(), k, comparator());
    }

    std::pair<iterator,iterator>
    equal_range(const key_type& k)
    {
      return std::equal_range(begin(), end(), k, comparator());
    }

    std::pair<const_iterator,const_iterator>
    equal_range(const key_type& k) const
    {
      return std::equal_range(begin(), end(), k, comparator());
    }


  // observers
    key_compare key_comp() const { return keycomp; }
    //~ using container::value_comp;

  // allocator
    allocator_type
    get_allocator() const noexcept
    {
      return base::get_allocator();
    }

  // additional public functions
    void persist(buffer_type& buffer)
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
            base::resize(size(buffer));

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
            base::resize(size(buffer));

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
    
    void persist()
    {
      persist(buf(this));
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



