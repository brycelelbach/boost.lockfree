////////////////////////////////////////////////////////////////////////////////
//  Algorithms from "CAS-Based Lock-Free Algorithm for Shared Deques"
//  by M. M. Michael
//  Link: http://www.research.ibm.com/people/m/michael/europar-2003.pdf 
//
//  C++ implementation - Copyright (C) 2011      Bryce Lelbach 
//  Test               - Copyright (C) 2009-2011 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  Disclaimer: Not a Boost library.
////////////////////////////////////////////////////////////////////////////////

#include <boost/thread/thread.hpp>
#include <boost/lockfree/deque.hpp>
#include <boost/program_options.hpp>

#include <hpx/util/lightweight_test.hpp>

boost::atomic<std::size_t> producer_count(0);
boost::atomic<std::size_t> left_producer_count(0);
boost::atomic<std::size_t> right_producer_count(0);

boost::atomic<std::size_t> consumer_count(0);
boost::atomic<std::size_t> left_consumer_count(0);
boost::atomic<std::size_t> right_consumer_count(0);

boost::lockfree::deque<std::size_t> deque;

std::size_t iterations = 100000;
std::size_t producer_thread_count = 4;
std::size_t consumer_thread_count = 4;

volatile bool done = false;

void producer()
{
    for (std::size_t i = 0; i != iterations; ++i) {
        std::size_t value = ++producer_count;

        if (producer_count & 1)
        {
            ++right_producer_count; 
            deque.push_right(value);
        }

        else
        {
            ++left_producer_count;
            deque.push_left(value);
        }
    }
}

bool consume(std::size_t& value)
{
    if (consumer_count & 1)
    {
        if (deque.pop_right(value))
        {
            ++consumer_count;
            ++right_consumer_count;
            return true;
        }
        return false;
    }

    else
    {
        if (deque.pop_left(value))
        {
            ++consumer_count;
            ++left_consumer_count;
            return true;
        }
        return false;
    }
}

void consumer()
{
    std::size_t value(0);
    while (!done) {
        while (consume(value)) {}
    }

    while (consume(value)) {}
}

int main(int argc, char** argv)
{
    using boost::program_options::variables_map;
    using boost::program_options::options_description;
    using boost::program_options::value;
    using boost::program_options::store;
    using boost::program_options::command_line_parser;
    using boost::program_options::notify;

    variables_map vm;

    options_description
        desc_cmdline("Usage: " HPX_APPLICATION_STRING " [options]");
    
    desc_cmdline.add_options()
        ("help,h", "print out program usage (this message)")
        ("producer-threads,p", value<std::size_t>(), 
         "the number of worker threads inserting objects into the deque "
         "(default: 4)") 
        ("consumer-threads,c", value<std::size_t>(), 
         "the number of worker threads removing objects into the deque "
         "(default: 4)") 
        ("iterations,i", value<std::size_t>(), 
         "the number of iterations (default: 100000)") 
    ;

    store(command_line_parser(argc, argv).options(desc_cmdline).run(), vm);

    notify(vm);

    // print help screen
    if (vm.count("help"))
    {
        std::cout << desc_cmdline;
        return hpx::util::report_errors();
    }

    if (vm.count("consumer-threads"))
        consumer_thread_count = vm["consumer-threads"].as<std::size_t>();
    
    if (vm.count("producer-threads"))
        producer_thread_count = vm["producer-threads"].as<std::size_t>();
    
    if (vm.count("iterations"))
        iterations = vm["iterations"].as<std::size_t>();

    std::cout << "boost::lockfree::deque is ";
    if (!deque.is_lock_free())
        std::cout << "not ";
    std::cout << "lockfree" << std::endl;

    boost::thread_group producer_threads, consumer_threads;

    for (std::size_t i = 0; i != producer_thread_count; ++i)
        producer_threads.create_thread(producer);

    for (std::size_t i = 0; i != consumer_thread_count; ++i)
        consumer_threads.create_thread(consumer);

    producer_threads.join_all();
    done = true;

    consumer_threads.join_all();

    HPX_TEST_EQ(left_producer_count + right_producer_count, producer_count);
    HPX_TEST_EQ(left_consumer_count + right_consumer_count, consumer_count);
    HPX_TEST_EQ(producer_count, consumer_count);

    std::cout << "produced " << producer_count << " objects in total:\n"
              << "         " << left_producer_count << " pushed on the left\n"
              << "         " << right_producer_count << " pushed on the right\n"
              << "consumed " << consumer_count << " objects in total:\n"
              << "         " << left_consumer_count << " popped on the left\n"
              << "         " << right_consumer_count << " popped on the right"
              << std::endl;

    return hpx::util::report_errors();
}

