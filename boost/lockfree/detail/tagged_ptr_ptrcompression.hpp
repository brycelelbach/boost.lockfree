//  tagged pointer, for aba prevention
//
//  Copyright (C) 2008, 2009 Tim Blechmann, based on code by Cory Nelson
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_TAGGED_PTR_PTRCOMPRESSION_HPP_INCLUDED
#define BOOST_LOCKFREE_TAGGED_PTR_PTRCOMPRESSION_HPP_INCLUDED

#include <boost/lockfree/detail/branch_hints.hpp>

#include <cstddef>              /* for std::size_t */

#include <boost/cstdint.hpp>

namespace boost
{
namespace lockfree
{

#if defined (__x86_64__) || defined (_M_X64) || defined(__alpha__)

template <class T>
struct tagged_ptr
{
    typedef boost::uint64_t compressed_ptr_t;
    typedef boost::uint16_t tag_t;

    union cast_unit
    {
        compressed_ptr_t value;
        tag_t tag[4];
    };

    BOOST_STATIC_CONSTANT(std::size_t, tag_index = 3);
    BOOST_STATIC_CONSTANT(compressed_ptr_t, ptr_mask = 0xffffffffffff);

    static T* extract_ptr(volatile compressed_ptr_t const & i)
    {
        return reinterpret_cast<T*>(i & ptr_mask);
    }

    static tag_t extract_tag(volatile compressed_ptr_t const & i)
    {
        cast_unit cu;
        cu.value = i;
        return cu.tag[tag_index];
    }

    static compressed_ptr_t pack_ptr(T * ptr, tag_t tag)
    {
        cast_unit ret;
        ret.value = reinterpret_cast<compressed_ptr_t>(ptr);
        ret.tag[tag_index] = tag;
        return ret.value;
    }

public:
    /** uninitialized constructor */
    tagged_ptr(void): 
        ptr(0)
    {}

    /** copy constructor */
    tagged_ptr(tagged_ptr const & p):
        ptr(p.ptr)
    {}

    explicit tagged_ptr(T * p):
        ptr(pack_ptr(p, 0))
    {}

    tagged_ptr(T * p, tag_t t):
        ptr(pack_ptr(p, t))
    {}

    /** unsafe set operation */
    /* @{ */
    tagged_ptr& operator= (tagged_ptr const & p)
    {
        ptr = p.ptr;
        return *this;
    }

    void set(T * p, tag_t t)
    {
        ptr = pack_ptr(p, t);
    }
    
    void reset(T * p, tag_t t)
    {
        set(p, t);
    } 
    /* @} */

    /** comparing semantics */
    /* @{ */
    bool operator== (volatile tagged_ptr const & p) const
    {
        return (ptr == p.ptr);
    }

    bool operator!= (volatile tagged_ptr const & p) const
    {
        return !operator==(p);
    }
    /* @} */

    /** pointer access */
    /* @{ */
    T * get_ptr() const volatile
    {
        return extract_ptr(ptr);
    }

    void set_ptr(T * p) volatile
    {
        tag_t tag = get_tag();
        ptr = pack_ptr(p, tag);
    }
    /* @} */

    /** tag access */
    /* @{ */
    tag_t get_tag() const volatile
    {
        return extract_tag(ptr);
    }

    void set_tag(tag_t t) volatile
    {
        T * p = get_ptr();
        ptr = pack_ptr(p, t);
    }
    /* @} */

    /** smart pointer support  */
    /* @{ */
    T & operator*() const
    {
        return *get_ptr();
    }

    T * operator->() const
    {
        return get_ptr();
    }

    operator bool(void) const
    {
        return get_ptr() != 0;
    }
    /* @} */

protected:
    compressed_ptr_t ptr;
};
#else
    #error unsupported platform
#endif

} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_TAGGED_PTR_PTRCOMPRESSION_HPP_INCLUDED */
