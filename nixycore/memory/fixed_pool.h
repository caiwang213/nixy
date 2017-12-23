/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/memory/alloc.h"
#include "nixycore/memory/std_alloc.h"

#include "nixycore/bugfix/assert.h"
#include "nixycore/pattern/iterator.h"
#include "nixycore/algorithm/series.h"

#include "nixycore/general/general.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

namespace use
{
    /*
        Memory Expand Models
    */

    template
    <
        class AllocT, size_t IterCountN,
        template <typename, size_t> class ModelT
    >
    struct pool_expand_keep
    {
        nx_assert_static(IterCountN);

    private:
        nx::iterator<ModelT<size_t, IterCountN> > count_ite_;

        struct blocks_t
        {
            pvoid     data_;
            size_t    size_;
            blocks_t* next_;
        } * blocks_head_;

    protected:
        pool_expand_keep(size_t init_count)
            : count_ite_(init_count)
            , blocks_head_(NULL)
        {}
        /* No return memory back to system */

        size_t count(void) const
        {
            return (*count_ite_);
        }

        pvoid expand(size_t block_size)
        {
            nx_assert(block_size)(block_size);
            blocks_t* blocks = nx::alloc<AllocT, blocks_t>();
            nx_assert(blocks);
            blocks->size_ = block_size * (*(++count_ite_));
            blocks->data_ = nx::alloc<AllocT>(blocks->size_);
            nx_assert(blocks->data_)(blocks->data_)(block_size);
            blocks->next_ = blocks_head_;
            blocks_head_ = blocks;
            return blocks->data_;
        }

    public:
        void clear(void)
        {
            while(blocks_head_)
            {
                blocks_t* blocks = blocks_head_;
                blocks_head_ = blocks_head_->next_;
                nx::free<AllocT>(blocks->data_, blocks->size_);
                nx::free<AllocT>(blocks);
            }
        }

        bool has_block(pvoid p) const
        {
            if (!p) return false;
            blocks_t* blocks = blocks_head_;
            while(blocks)
            {
                if ((blocks->data_ <= p) &&
                    (p < ((nx::byte*)(blocks->data_) + blocks->size_)))
                    return true;
                blocks = blocks->next_;
            }
            return false;
        }
    };

    template
    <
        class AllocT, size_t IterCountN,
        template <typename, size_t> class ModelT
    >
    struct pool_expand_return : pool_expand_keep<AllocT, IterCountN, ModelT>
    {
    private:
        typedef pool_expand_keep<AllocT, IterCountN, ModelT> base_t;

    protected:
        pool_expand_return(size_t init_count)
            : base_t(init_count)
        {}
        ~pool_expand_return(void)
        { base_t::clear(); }
    };
}

/*
    Default allocation parameters
*/

#ifndef NX_FIXEDPOOL_EXPAND
#define NX_FIXEDPOOL_EXPAND     nx::use::pool_expand_return // Memory expand model using pool_expand_return
#endif

#ifndef NX_FIXEDPOOL_MODEL
#define NX_FIXEDPOOL_MODEL      nx::use::iter_powerof       // Iterative algorithm using iter_powerof
#endif

#ifndef NX_FIXEDPOOL_ITERCOUNT
#define NX_FIXEDPOOL_ITERCOUNT  (2)                         // Iteration factor
#endif

/*
    Fixed Memory Pool
*/

template
<
    class AllocT = use::alloc_std,

    template
    <
        class, size_t,
        template <typename, size_t> class
    >
    class ExpandT = NX_FIXEDPOOL_EXPAND,

    template <typename, size_t>
    class ModelT = NX_FIXEDPOOL_MODEL,

    size_t IterCountN = NX_FIXEDPOOL_ITERCOUNT
>
class fixed_pool : public ExpandT<AllocT, IterCountN, ModelT>, noncopyable
{
public:
    typedef ExpandT<AllocT, IterCountN, ModelT> base_t;

private:
    size_t block_size_;
    pvoid cursor_;

    void expand(void)
    {
        pvoid* p = (pvoid*)(cursor_ = base_t::expand(block_size_));
        nx_assert(p);
        for(size_t i = 0; i < base_t::count() - 1; ++i)
            p = (pvoid*)( (*p) = ((byte*)p) + block_size() );
        (*p) = NULL;
    }

public:
    explicit fixed_pool(size_t block_size, size_t init_count = 0)
        : base_t(init_count)
        , block_size_(block_size)
        , cursor_(0)
    {
        nx_assert(block_size_ >= sizeof(pvoid))(block_size_);
    }

    size_t block_size(void) const { return block_size_; }

    pvoid alloc(void)
    {
        if (!cursor_) expand();
        nx_assert(cursor_);
        pvoid p = cursor_;
        cursor_ = *(pvoid*)p;   // Get next block
        return p;
    }

    void free(pvoid p)
    {
        if (!p) return;
        *(pvoid*)p = cursor_;
        cursor_ = p;            // Save free block
    }

    using base_t::clear;
    using base_t::has_block;
};

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
