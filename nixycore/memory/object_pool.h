/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/memory/fixed_pool.h"
#include "nixycore/memory/construct.h"

#include "nixycore/delegate/functor.h"
#include "nixycore/delegate/bind.h"

#include "nixycore/bugfix/assert.h"

#include "nixycore/general/general.h"
#include "nixycore/preprocessor/preprocessor.h"
#include "nixycore/typemanip/typemanip.h"
#include "nixycore/utility/utility.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

/*
    the data structure for storing objects
*/

template <typename T, class FixedAllocT>
class object_pool_storage
{
public:
    typedef T type_t;

protected:
    FixedAllocT allocator_;
    functor<type_t*(pvoid)> constructor_;

    struct alloc_t
    {
        nx::byte data_[sizeof(type_t)];
        alloc_t* next_;
    };

    alloc_t* free_head_;
    alloc_t* free_tail_;

    size_t size_;

public:
    template <typename F>
    explicit object_pool_storage(const F& f)
        : allocator_(sizeof(alloc_t))
        , constructor_(f)
        , free_head_(NULL)
        , free_tail_(NULL)
        , size_(0)
    {}

    ~object_pool_storage(void)
    {
        while (!is_empty()) destroy(take());
    }

public:
    type_t* create(void)
    {
        alloc_t* p = static_cast<alloc_t*>(allocator_.alloc());
        nx_assert(p);
        return constructor_(p->data_);
    }

    void destroy(type_t* objc)
    {
        nx_assert(objc);
        nx_destruct(objc, type_t);
        allocator_.free(objc);
    }

    type_t* take(void)
    {
        nx_assert(free_head_);
        alloc_t* p = free_head_;
        free_head_ = free_head_->next_;
        if (free_tail_ == p)
            free_tail_ = free_head_ = nx::nulptr;
        -- size_;
        return reinterpret_cast<type_t*>(p->data_);
    }

    void put(type_t* objc)
    {
        nx_assert(objc);
        alloc_t* p = reinterpret_cast<alloc_t*>(objc);
        if ( free_tail_) free_tail_->next_ = p;
        free_tail_ = p;
        if (!free_head_) free_head_ = free_tail_;
        ++ size_;
    }

    size_t size(void) const
    {
        return size_;
    }

    bool is_empty(void) const
    {
        return (size_ == 0);
    }
};

/*
    the function templates for constructing object
*/

namespace private_object_pool
{
#ifdef NX_SP_CXX11_TEMPLATES
    template <typename T, typename... P>
    struct invoker
    {
        static T* invoke(pvoid p, P... par)
        {
            return nx_construct(p, T, (nx_forward(P, par)...));
        }
    };
#else /*NX_SP_CXX11_TEMPLATES*/
    template <typename T, typename F>
    struct invoker;

    template <typename T>
    struct invoker<T, void()>
    {
        static T* invoke(pvoid p)
        {
            return nx_construct(p, T);
        }
    };

#define NX_OBJECT_POOL_HELPER_(n) \
    template <typename T, NX_PP_TYPE_1(n, typename P)> \
    struct invoker<T, void(NX_PP_TYPE_1(n, P))> \
    { \
        static T* invoke(pvoid p, NX_PP_TYPE_2(n, P, par)) \
        { \
            return nx_construct(p, T, (NX_PP_FORWARD(n, P, par))); \
        } \
    };
    NX_PP_MULT_MAX(NX_OBJECT_POOL_HELPER_)
#undef NX_OBJECT_POOL_HELPER_
#endif/*NX_SP_CXX11_TEMPLATES*/
}

/*
    the object pool
*/

template
<
    typename T,
    class FixedAllocT = fixed_pool<>,
    template <typename, class>
    class StorageT = object_pool_storage
>
class object_pool : noncopyable
{
    typedef StorageT<T, FixedAllocT> storage_t;

public:
    typedef typename storage_t::type_t type_t;

protected:
    storage_t storage_;

    size_t min_size_;
    size_t max_size_;

public:
#ifdef NX_SP_CXX11_TEMPLATES
    template <typename... P>
    object_pool(nx_fref(P)... par)
        : storage_(bind(&private_object_pool::invoker<type_t, P...>::invoke,
                         nx::_1, nx_forward(P, par)...))
        , min_size_(0)
        , max_size_(0)
    {
        limit();
    }
#else /*NX_SP_CXX11_TEMPLATES*/
    object_pool(void)
        : storage_(&private_object_pool::invoker<type_t, void()>::invoke)
        , min_size_(0)
        , max_size_(0)
    {
        limit();
    }

#define NX_OBJECT_POOL_(n) \
    template <NX_PP_TYPE_1(n, typename P)> \
    object_pool(NX_PP_TYPE_2(n, P, NX_PP_FREF(par))) \
        : storage_(bind(&private_object_pool::invoker<type_t, void(NX_PP_TYPE_1(n, P))>::invoke, \
                         nx::_1, NX_PP_FORWARD(n, P, par))) \
        , min_size_(0) \
        , max_size_(0) \
    { \
        limit(); \
    }
    NX_PP_MULT_MAX(NX_OBJECT_POOL_)
#undef NX_OBJECT_POOL_
#endif/*NX_SP_CXX11_TEMPLATES*/

public:
    void limit(size_t min_sz = 0, size_t max_sz = (size_t)~0)
    {
        nx_assert(max_sz);
        nx_assert(min_sz <= max_sz);
        min_size_ = min_sz;
        max_size_ = max_sz;
        if (storage_.size() > max_size_)
        {
            do
            {
                nx_verify(decrease());
            }
            while (storage_.size() > max_size_);
        }
        else
        {
            while (storage_.size() < min_size_)
            {
                nx_verify(increase());
            }
        }
    }

    size_t min_size(void) const
    {
        return min_size_;
    }

    size_t max_size(void) const
    {
        return max_size_;
    }

    size_t size(void) const
    {
        return storage_.size();
    }

    bool is_empty(void) const
    {
        return storage_.is_empty();
    }

    bool is_full(void) const
    {
        return (storage_.size() >= max_size_);
    }

    bool is_thin(void) const
    {
        return (storage_.size() <= min_size_);
    }

    bool is_over(void) const
    {
        return (storage_.size() > max_size_);
    }

    bool is_lack(void) const
    {
        return (storage_.size() < min_size_);
    }

public:
    bool increase(void)
    {
        if (is_full()) return false;
        storage_.put(storage_.create());
        return true;
    }

    bool decrease(void)
    {
        if (is_thin()) return false;
        storage_.destroy(storage_.take());
        return true;
    }

    type_t* alloc(void)
    {
        if (is_empty()) increase();
        return storage_.take();
    }

    void free(type_t* objc)
    {
        if (!objc) return;
        if (is_full())
            storage_.destroy(objc);
        else
            storage_.put(objc);
    }

    void clear(void)
    {
        while (decrease()) continue;
    }
};

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
