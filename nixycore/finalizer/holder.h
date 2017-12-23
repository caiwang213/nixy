/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/finalizer/ref_counter.h"

#include "nixycore/general/general.h"
#include "nixycore/typemanip/typemanip.h"
#include "nixycore/utility/utility.h"
#include "nixycore/algorithm/algorithm.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

namespace private_holder
{
    template <typename T, class P, class AllocT, class ModelT>
    class detail : public ref_base<AllocT, ModelT>, public P
    {
        typedef ref_base<AllocT, ModelT> base_t;

    public:
        typedef T      type_t;
        typedef P      policy_t;
        typedef ModelT model_t;

    protected:
        type_t res_;

        template <typename F>
        void assign_to(const type_t& r, nx_fref(F) dest_fr)
        {
            if (policy_t::is_valid(r))
            {
                base_t::init(nx_forward(F, dest_fr));
                res_ = r;
            }
        }

    public:
        detail(void)
        {
            policy_t::reset(res_);
        }

    public:
        void set(const type_t& r)
        {
            assign_to(r, nx_pass(make_destructor(r)));
        }

        template <typename F>
        void set(const type_t& r, nx_fref(F) dest_fr)
        {
            assign_to(r, nx_pass(make_destructor(r, nx_forward(F, dest_fr))));
        }

        template <typename U>
        void set(const detail<U, P, AllocT, ModelT>& r)
        {
            if (base_t::set(r))
                res_ = (type_t)r.get();
            else
                policy_t::reset(res_);
        }

        void swap(detail& rhs)
        {
            base_t::swap(rhs);
            nx::swap(res_, rhs.res_);
        }

        type_t get(void) const { return res_; }
    };
}

/*
    Using reference counting to manage resources
*/

template <typename T, class P, class AllocT = NX_DEFAULT_ALLOC, class ModelT = NX_DEFAULT_THREAD_MODEL>
class holder
    : public ref_counter<private_holder::detail<T, P, AllocT, ModelT> >
    , public safe_bool<holder<T, P, AllocT, ModelT> >
{
    typedef ref_counter<private_holder::detail<T, P, AllocT, ModelT> > base_t;

public:
    holder(void)
        : base_t()
    {}

    template <typename U>
    holder(nx_fref(U) r)
        : base_t(nx_forward(U, r))
    {}

    template <typename U, typename F>
    holder(nx_fref(U) r, nx_fref(F) dest_fr)
        : base_t(nx_forward(U, r), nx_forward(F, dest_fr))
    {}

    holder(const holder& r)
        : base_t(static_cast<const base_t&>(r))
    {}

    holder(nx_rref(holder) r)
        : base_t()
    { swap(nx::moved(r)); }

    holder& operator=(holder rhs)
    {
        rhs.swap(*this);
        return (*this);
    }

    bool check_safe_bool(void) const
    { return base_t::is_valid(base_t::get()); }

    void swap(holder& rhs) { base_t::swap(static_cast<base_t&>(rhs)); }
};

/*
    Special swap algorithm
*/

template <typename T, class P, class A, class M>
inline void swap(holder<T, P, A, M>& x, holder<T, P, A, M>& y)
{
    x.swap(y);
}

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
