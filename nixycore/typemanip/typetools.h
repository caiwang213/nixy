/*
    The Nixy Library
    Code covered by the MIT License

    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include "nixycore/typemanip/typedefs.h"

#include "nixycore/general/general.h"

//////////////////////////////////////////////////////////////////////////
NX_BEG
//////////////////////////////////////////////////////////////////////////

/*
    Type Wrappers
*/

template <typename T>
struct type_wrap
{
    typedef T type_t;
};

template <int N>
struct type_int
{
    NX_STATIC_VALUE(int, N);
};

/*
    For judge bool value
*/

template <bool>
struct type_if;

template <>
struct type_if<true> : true_t
{
    typedef true_t type_t;
};

template <>
struct type_if<false> : false_t
{
    typedef false_t type_t;
};

/*
    For select a type
*/

template <bool, typename T, typename U>
struct select_if
{
    typedef T type_t;
};

template <typename T, typename U>
struct select_if<false, T, U>
{
    typedef U type_t;
};

/*
    Enable if (bool == true)
*/

template <bool, typename T = void>
struct enable_if
{
    // Nothing
};

template <typename T>
struct enable_if<true, T> 
{ 
    typedef T type_t; 
};

/*
    Shield to protect the parameters are not destroyed by macro expansion

    #define MACRO_VALUE(x)  x::value
    -->
    MACRO_VALUE(TemplateClass<A, B>)                // Error!
    MACRO_VALUE(NX_SHIELD(TemplateClass<A, B>))     // OK~
*/

template <typename T> struct strip          { typedef T type_t; };
template <typename T> struct strip<void(T)> { typedef T type_t; };

#define NX_SHIELD(...) nx::strip<void(__VA_ARGS__)>::type_t

//////////////////////////////////////////////////////////////////////////
NX_END
//////////////////////////////////////////////////////////////////////////
