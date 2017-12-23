/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/general/general.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct series_base
{
    typedef T type_t;

    type_t val_;

    series_base(type_t val) : val_(val) {}

    type_t value(void) const { return val_; }

    bool operator==(const series_base& y) const
    {
        return val_ == y.val_;
    }
};

namespace use // Series Policy
{
    template <typename T, size_t /*N*/>
    struct iter_const : series_base<T>
    {
        typedef series_base<T> base_t;
        typedef typename base_t::type_t type_t;

        iter_const(type_t val)
            : base_t(val)
        {
            if (base_t::val_ == 0)
                base_t::val_ = 1;
        }

        void operator()(int /*n*/)
        {
            // Do nothing
        }
    };

    /*
        Accumulation (N * (n - 1))

        -->
        0 : 0
        n : N + f(n - 1)
    */

    template <typename T, size_t N>
    struct iter_acc : series_base<T>
    {
        typedef series_base<T> base_t;
        typedef typename base_t::type_t type_t;

        iter_acc(type_t val)
            : base_t(val)
        {}

        void operator()(int n)
        {
            if (n == 0) return;
            if (n > 0)
                base_t::val_ += (N * n);
            else
                base_t::val_ -= (N * (-n));
        }
    };

    /*
        (N ^ (n - 1))

        -->
        0 : 0
        1 : 1
        n : N * f(n - 1)
    */

    template <typename T, size_t N>
    struct iter_powerof : series_base<T>
    {
        typedef series_base<T> base_t;
        typedef typename base_t::type_t type_t;

        iter_powerof(type_t val)
            : base_t(val)
        {}

        void operator()(int n)
        {
            if (n == 0) return;
            if (n > 0)
            {
                if (base_t::val_ == 0)
                {
                    base_t::val_ = 1;
                    --n;
                }
                for(int i = 0; i < n; ++i)
                {
                    base_t::val_ *= N;
                }
            }
            else
            {
                for(int i = 0; i > n; --i)
                {
                    // when val == 1, val / N == 0
                    base_t::val_ /= N;
                }
            }
        }
    };

    /*
        Fibonacci

        -->
        0 : 0
        1 : N
        n : f(n - 1) + f(n - 2)
    */

    template <typename T, size_t N = 1>
    struct iter_fibonacci : series_base<T>
    {
        typedef series_base<T> base_t;
        typedef typename base_t::type_t type_t;

        type_t prv_;

        iter_fibonacci(type_t val)
            : base_t(val)
            , prv_(0)
        {}

        void operator()(int n)
        {
            if (n == 0) return;
            if (n > 0) for(int i = 0; i < n; ++i)
            {
                if (base_t::val_ == 0)
                {
                    base_t::val_ = N;
                    prv_ = 0;
                }
                else
                {
                    T tmp = base_t::val_;
                    base_t::val_ += prv_;
                    prv_ = tmp;
                }
            }
            else for(int i = 0; i > n; --i)
            {
                T tmp = prv_;
                prv_ = (base_t::val_ -= prv_);
                base_t::val_ = tmp;
            }
        }
    };
}

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
