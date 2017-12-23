/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/thread/atomic.h"
#include "nixycore/thread/spin_lock.h"
#include "nixycore/thread/mutex.h"

#include "nixycore/general/general.h"
#include "nixycore/utility/utility.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

template <class ModelT>
struct thread_model : ModelT
{
    template <typename T>
    struct atomic { typedef nx::atomic<T, typename ModelT::interlocked> type_t; };
};

/*
    non_lock, for single-thread model
*/

class non_lock : nx::noncopyable
{
public:
    typedef non_lock lock_t;
    typedef lock_t   handle_t;
    handle_t&       operator*(void)       { return (*this); }
    const handle_t& operator*(void) const { return (*this); }

public:
    bool try_lock(void) { return true; }
    void lock    (void) {}
    void unlock  (void) {}
};

/*
    single-thread model
*/

struct single_thread_model
{
    typedef use::interlocked_st interlocked;
    typedef non_lock lock_t;
    typedef non_lock mutex_t;
};

/*
    multi-thread model
*/

struct multi_thread_model
{
    typedef use::interlocked_mt interlocked;
    typedef spin_lock lock_t;
    typedef mutex     mutex_t;
};

/*
    default thread model
*/

namespace use
{
    typedef thread_model<single_thread_model> thread_single;
    typedef thread_model<multi_thread_model>  thread_multi;
}

#ifndef NX_DEFAULT_THREAD_MODEL
#   ifdef NX_SINGLE_THREAD
#       define NX_DEFAULT_THREAD_MODEL  nx::use::thread_single
#   else
#       define NX_DEFAULT_THREAD_MODEL  nx::use::thread_multi
#   endif
#endif

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
