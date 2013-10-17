/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace mstd {

template<template<class Dummy> class T, class T1>
T<T1> auto_bind(const T1 & t1)
{
    return T<T1>(t1);
}

template<template<class D1, class D2> class T, class T1, class T2>
T<T1, T2> auto_bind(const T1 & t1, const T2 & t2)
{
    return T<T1, T2>(t1, t2);
}

}
