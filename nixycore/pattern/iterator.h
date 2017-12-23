/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/general/general.h"
#include "nixycore/utility/utility.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

template <typename ModelT>
struct iterator : public ModelT, nx_operator(iterator<ModelT>, unequal, operable)
{
    typedef typename ModelT::type_t type_t;

    iterator(void)
        : ModelT(0)
    {}

    template <typename T>
    iterator(T v)
        : ModelT(v)
    {}

    iterator& operator+=(int v)
    {
        ModelT::operator()(v);
        return (*this);
    }

    bool operator==(const iterator& y) const
    {
        return ModelT::operator==(y);
    }

    type_t value(void) const
    {
        return ModelT::value();
    }

    type_t operator*(void) const { return value(); }
};

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
