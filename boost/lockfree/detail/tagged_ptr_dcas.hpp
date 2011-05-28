//  tagged pointer, for aba prevention
//
//  Copyright (C) 2008 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_TAGGED_PTR_DCAS_HPP_INCLUDED
#define BOOST_LOCKFREE_TAGGED_PTR_DCAS_HPP_INCLUDED

#include <boost/lockfree/detail/branch_hints.hpp>

#include <cstddef>              /* for std::size_t */

namespace boost
{
namespace lockfree
{

template <class T>
struct BOOST_LOCKFREE_DCAS_ALIGNMENT tagged_ptr
{
    typedef std::size_t tag_t;

    /** uninitialized constructor */
    tagged_ptr(void)//: ptr(0), tag(0)
    {}

    tagged_ptr(tagged_ptr const & p):
        ptr(p.ptr), tag(p.tag)
    {}

    explicit tagged_ptr(T * p):
        ptr(p), tag(0)
    {}

    tagged_ptr(T * p, tag_t t):
        ptr(p), tag(t)
    {}

    /** unsafe set operation */
    /* @{ */
    tagged_ptr& operator= (tagged_ptr const & p)
    {
        set(p.ptr, p.tag);
        return *this;
    }

    void set(T * p, tag_t t)
    {
        ptr = p;
        tag = t;
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
        return (ptr == p.ptr) && (tag == p.tag);
    }

    bool operator!= (volatile tagged_ptr const & p) const
    {
        return !operator==(p);
    }
    /* @} */

    /** pointer access */
    /* @{ */
    T * get_ptr(void) const volatile
    {
        return ptr;
    }

    void set_ptr(T * p) volatile
    {
        ptr = p;
    }
    /* @} */

    /** tag access */
    /* @{ */
    tag_t get_tag() const volatile
    {
        return tag;
    }

    void set_tag(tag_t t) volatile
    {
        tag = t;
    }
    /* @} */

    /** smart pointer support  */
    /* @{ */
    T & operator*() const
    {
        return *ptr;
    }

    T * operator->() const
    {
        return ptr;
    }

    operator bool(void) const
    {
        return ptr != 0;
    }
    /* @} */

protected:
    T * ptr;
    tag_t tag;
};

} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_TAGGED_PTR_DCAS_HPP_INCLUDED */
