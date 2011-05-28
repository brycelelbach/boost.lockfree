////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2011 Bryce Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#if !defined(HPX_F646702C_6556_48FA_BF9D_3E7959983122)
#define HPX_F646702C_6556_48FA_BF9D_3E7959983122

#include <cstddef>

#include <iostream>

#include <boost/io/ios_state.hpp>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/current_function.hpp>
#include <boost/preprocessor/stringize.hpp>

// Use smart_ptr's spinlock header because this header is used by the CMake
// config tests, and therefore we can't include other hpx headers in this file.
#include <boost/smart_ptr/detail/spinlock.hpp>

namespace hpx { namespace util
{ 

enum counter_type
{
    counter_sanity,
    counter_test
};

namespace detail
{

struct fixture
{
  public:
    typedef boost::detail::spinlock mutex_type;

  private:
    std::ostream& stream_;
    std::size_t sanity_failures_;
    std::size_t test_failures_;
    mutex_type mutex_;

  public:
    fixture(std::ostream& stream):
      stream_(stream), sanity_failures_(0), test_failures_(0)
    { 
        mutex_type l = BOOST_DETAIL_SPINLOCK_INIT;
        mutex_ = l;
    }

    void increment(counter_type c)
    {
        switch (c)
        {
            case counter_sanity:
                ++sanity_failures_; return;
            case counter_test:
                ++test_failures_; return;
            default:
                { BOOST_ASSERT(false); return; }
        }
    }

    std::size_t get(counter_type c) const
    {
        switch (c)
        {
            case counter_sanity:
                return sanity_failures_;
            case counter_test:
                return test_failures_;
            default:
                { BOOST_ASSERT(false); return 0; }
        }
    }

    template <typename T>
    bool check(char const* file, int line, char const* function,
               counter_type c, T const& t, char const* msg)
    {
        if (!t)
        { 
            mutex_type::scoped_lock l(mutex_);
            boost::io::ios_flags_saver ifs(stream_); 
            stream_ 
                << file << "(" << line << "): "
                << msg << " failed in function '"
                << function << "'" << std::endl;
            increment(c);
            return false;
        }
        return true;
    }

    template <typename T, typename U>
    bool check_equal(char const* file, int line, char const* function,
                  counter_type c, T const& t, U const& u, char const* msg)
    {
        if (!(t == u))
        {
            mutex_type::scoped_lock l(mutex_);
            boost::io::ios_flags_saver ifs(stream_); 
            stream_ 
                << file << "(" << line << "): " << msg  
                << " failed in function '" << function << "': "
                << "'" << t << "' != '" << u << "'" << std::endl;
            increment(c);
            return false;
        }
        return true;
    }
    
    template <typename T, typename U>
    bool check_not_equal(char const* file, int line, char const* function,
                         counter_type c, T const& t, U const& u,
                         char const* msg)
    {
        if (!(t != u))
        {
            mutex_type::scoped_lock l(mutex_);
            boost::io::ios_flags_saver ifs(stream_); 
            stream_ 
                << file << "(" << line << "): " << msg  
                << " failed in function '" << function << "': "
                << "'" << t << "' != '" << u << "'" << std::endl;
            increment(c);
            return false;
        }
        return true;
    }

    template <typename T, typename U>
    bool check_less(char const* file, int line, char const* function,
                    counter_type c, T const& t, U const& u, char const* msg)
    {
        if (!(t < u))
        {
            mutex_type::scoped_lock l(mutex_);
            boost::io::ios_flags_saver ifs(stream_); 
            stream_ 
                << file << "(" << line << "): " << msg  
                << " failed in function '" << function << "': "
                << "'" << t << "' >= '" << u << "'" << std::endl;
            increment(c);
            return false;
        }
        return true;
    }
    
    template <typename T, typename U>
    bool check_less_equal(char const* file, int line, char const* function,
                          counter_type c, T const& t, U const& u,
                          char const* msg)
    {
        if (!(t <= u))
        {
            mutex_type::scoped_lock l(mutex_);
            boost::io::ios_flags_saver ifs(stream_); 
            stream_ 
                << file << "(" << line << "): " << msg  
                << " failed in function '" << function << "': "
                << "'" << t << "' > '" << u << "'" << std::endl;
            increment(c);
            return false;
        }
        return true;
    }
};

fixture global_fixture = fixture(std::cerr);

} // hpx::util::detail

inline int report_errors(std::ostream& stream = std::cerr)
{
    std::size_t sanity = detail::global_fixture.get(counter_sanity),
                test   = detail::global_fixture.get(counter_test); 
    if (sanity == 0 && test == 0)
        return 0;

    else
    {
        boost::io::ios_flags_saver ifs(stream); 
        stream << sanity << " sanity check"
               << ((sanity == 1) ? " and " : "s and ")
               << test << " test"
               << ((test == 1) ? " failed." : "s failed.")
               << std::endl;
        return 1;
    }
}

}} // hpx::util

#define HPX_TEST(expr)                                                      \
    ::hpx::util::detail::global_fixture.check                               \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr, "test '" BOOST_PP_STRINGIZE(expr) "'")                       \
    /***/

#define HPX_TEST_MSG(expr, msg)                                             \
    ::hpx::util::detail::global_fixture.check                               \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr, msg)                                                         \
    /***/

#define HPX_TEST_EQ(expr1, expr2)                                           \
    ::hpx::util::detail::global_fixture.check_equal                         \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr1, expr2, "test '" BOOST_PP_STRINGIZE(expr1) " == "            \
                                BOOST_PP_STRINGIZE(expr2) "'")              \
    /***/

#define HPX_TEST_NEQ(expr1, expr2)                                          \
    ::hpx::util::detail::global_fixture.check_not_equal                     \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr1, expr2, "test '" BOOST_PP_STRINGIZE(expr1) " != "            \
                                BOOST_PP_STRINGIZE(expr2) "'")              \
    /***/

#define HPX_TEST_LT(expr1, expr2)                                           \
    ::hpx::util::detail::global_fixture.check_less                          \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr1, expr2, "test '" BOOST_PP_STRINGIZE(expr1) " < "             \
                                BOOST_PP_STRINGIZE(expr2) "'")              \
    /***/

#define HPX_TEST_LTE(expr1, expr2)                                          \
    ::hpx::util::detail::global_fixture.check_less_equal                    \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr1, expr2, "test '" BOOST_PP_STRINGIZE(expr1) " <= "            \
                                BOOST_PP_STRINGIZE(expr2) "'")              \
    /***/

#define HPX_TEST_EQ_MSG(expr1, expr2, msg)                                  \
    ::hpx::util::detail::global_fixture.check_equal                         \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_test,                                         \
         expr1, expr2, msg)                                                 \
    /***/

#define HPX_SANITY(expr)                                                    \
    ::hpx::util::detail::global_fixture.check                               \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr, "sanity check '" BOOST_PP_STRINGIZE(expr) "'")               \
    /***/

#define HPX_SANITY_MSG(expr, msg)                                           \
    ::hpx::util::detail::global_fixture.check                               \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr, msg)                                                         \
    /***/

#define HPX_SANITY_EQ(expr1, expr2)                                         \
    ::hpx::util::detail::global_fixture.check_equal                         \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr1, expr2, "sanity check '" BOOST_PP_STRINGIZE(expr1) " == "    \
                                        BOOST_PP_STRINGIZE(expr2) "'")      \
    /***/

#define HPX_SANITY_NEQ(expr1, expr2)                                        \
    ::hpx::util::detail::global_fixture.check_not_equal                     \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr1, expr2, "sanity check '" BOOST_PP_STRINGIZE(expr1) " != "    \
                                        BOOST_PP_STRINGIZE(expr2) "'")      \
    /***/

#define HPX_SANITY_LT(expr1, expr2)                                         \
    ::hpx::util::detail::global_fixture.check_less                          \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr1, expr2, "sanity check '" BOOST_PP_STRINGIZE(expr1) " < "     \
                                        BOOST_PP_STRINGIZE(expr2) "'")      \
    /***/

#define HPX_SANITY_LTE(expr1, expr2)                                        \
    ::hpx::util::detail::global_fixture.check_less_equal                    \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr1, expr2, "sanity check '" BOOST_PP_STRINGIZE(expr1) " <= "    \
                                        BOOST_PP_STRINGIZE(expr2) "'")      \
    /***/

#define HPX_SANITY_EQ_MSG(expr1, expr2, msg)                                \
    ::hpx::util::detail::global_fixture.check_equal                         \
        (__FILE__, __LINE__, BOOST_CURRENT_FUNCTION,                        \
         ::hpx::util::counter_sanity,                                       \
         expr1, expr2)                                                      \
    /***/

#endif // HPX_F646702C_6556_48FA_BF9D_3E7959983122

