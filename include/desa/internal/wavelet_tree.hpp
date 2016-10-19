/************************************************
 *  wavelet_tree.hpp
 *  DESA
 *
 *  Copyright (c) 2015, Chi-En Wu
 *  Distributed under The BSD 3-Clause License
 ************************************************/

#ifndef DESA_INTERNAL_WAVELET_TREE_HPP_
#define DESA_INTERNAL_WAVELET_TREE_HPP_

#include <climits>
#include <array>
#include <utility>

#include "bit_vector.hpp"
#include "partial_sum.hpp"

namespace desa
{

namespace internal
{

/************************************************
 * Declaration: class wavelet_tree<T, N>
 ************************************************/

template <typename T, ::std::size_t N = sizeof(T) * CHAR_BIT>
class wavelet_tree
{
public: // Public Type(s)
    typedef T value_type;
    typedef ::std::size_t size_type;

public: // Public Method(s)
    void insert(size_type i, value_type c);
    value_type erase(size_type i);

    size_type size(void) const;

    size_type sum(value_type c) const;
    value_type search(size_type i) const;
    ::std::pair<value_type, size_type> access_and_rank(size_type i) const;
    size_type rank(size_type i, value_type c) const;
    size_type select(size_type j, value_type c) const;

    ::std::pair<value_type, size_type> access_and_lf(size_type i) const;
    size_type lf(size_type i) const;
    size_type psi(size_type i) const;

    value_type at(size_type i) const;

    value_type operator[](size_type i) const;

private: // Private Static Property(ies)
    static constexpr size_type HEIGHT = N;
    static constexpr size_type BITMAP_BLOCK_SIZE = 64;

private: // Private Type(s)
    typedef bit_vector<BITMAP_BLOCK_SIZE> bitmap;
    typedef ::std::pair<size_type, bitmap> tree_level;

private: // Private Method(s)
    void increase_num_zeros(size_type l);
    void decrease_num_zeros(size_type l);
    size_type num_zeros(size_type l) const;
    bitmap &level_bits(size_type l);
    bitmap const &level_bits(size_type l) const;
    size_type select_at(size_type j, value_type c) const;

private: // Private Property(ies)
    ::std::array<tree_level, N> levels_;
   partial_sum<value_type, size_type> sums_;
}; // class wavelet_tree<T, N>

/************************************************
 * Implementation: class wavelet_tree<T, N>
 ************************************************/

template <typename T, ::std::size_t N>
void wavelet_tree<T, N>::insert(size_type i, value_type c)
{
    sums_.increase(c, 1);
    for (size_type l = 0; l < HEIGHT; ++l, c >>= 1)
    {
        auto &bits = level_bits(l);
        auto b = c & 1;
        bits.insert(i, b);
        i = bits.rank(i, b) - 1;
        if (b)  { i += num_zeros(l); }
        else    { increase_num_zeros(l); }
    }
}

template <typename T, ::std::size_t N>
typename wavelet_tree<T, N>::value_type wavelet_tree<T, N>::erase(size_type i)
{
    value_type c = 0;
    for (size_type l = 0; l < HEIGHT; ++l)
    {
        auto &bits = level_bits(l);
        auto b = bits.erase(i);
        i = (i > 0) ? bits.rank(i - 1, b) : 0;
        c |= b << l;
        if (b)  { i += num_zeros(l); }
        else    { decrease_num_zeros(l); }
    }

    sums_.decrease(c, 1);
    return c;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::size(void) const
{
    auto const &bits = level_bits(0);
    return bits.size();
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::sum(value_type c) const
{
    return (c > 0) ? sums_.sum(c - 1) : 0;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::value_type wavelet_tree<T, N>::search(size_type i) const
{
    return sums_.search(i);
}

template <typename T, ::std::size_t N>
inline ::std::pair<typename wavelet_tree<T, N>::value_type, typename wavelet_tree<T, N>::size_type>
wavelet_tree<T, N>::access_and_rank(size_type i) const
{
    auto pair = access_and_lf(i);
    auto ps = sum(pair.first);
    return ::std::make_pair(pair.first, pair.second + 1 - ps);
}

template <typename T, ::std::size_t N>
typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::rank(size_type i, value_type c) const
{
    auto ps = sum(c);
    for (size_type l = 0; l < HEIGHT; ++l, c >>= 1)
    {
        auto &bits = level_bits(l);
        auto b = c & 1;
        i = bits.rank(i, b);
        if (i > 0)  { --i; }
        else        { return 0; }

        i += b ? num_zeros(l) : 0;
    }

    return i + 1 - ps;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::select(size_type j, value_type c) const
{
    return select_at(j + sum(c), c);
}

template <typename T, ::std::size_t N>
::std::pair<typename wavelet_tree<T, N>::value_type, typename wavelet_tree<T, N>::size_type>
wavelet_tree<T, N>::access_and_lf(size_type i) const
{
    value_type c = 0;
    for (size_type l = 0; l < HEIGHT; ++l)
    {
        auto &bits = level_bits(l);
        auto br_pair = bits.access_and_rank(i);
        c |= br_pair.first << l;
        i = (br_pair.first ? num_zeros(l) : 0) + br_pair.second - 1;
    }

    return ::std::make_pair(c, i);
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::lf(size_type i) const
{
    return access_and_lf(i).second;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::psi(size_type i) const
{
    auto c = sums_.search(i + 1);
    return select_at(i, c);
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::value_type wavelet_tree<T, N>::at(size_type i) const
{
    return access_and_lf(i).first;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::value_type wavelet_tree<T, N>::operator[](size_type i) const
{
    return at(i);
}

template <typename T, ::std::size_t N>
inline void wavelet_tree<T, N>::increase_num_zeros(size_type l)
{
    levels_[l].first++;
}

template <typename T, ::std::size_t N>
inline void wavelet_tree<T, N>::decrease_num_zeros(size_type l)
{
    levels_[l].first--;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::num_zeros(size_type l) const
{
    return levels_[l].first;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::bitmap &wavelet_tree<T, N>::level_bits(size_type l)
{
    return levels_[l].second;
}

template <typename T, ::std::size_t N>
inline typename wavelet_tree<T, N>::bitmap const &wavelet_tree<T, N>::level_bits(size_type l) const
{
    return levels_[l].second;
}

template <typename T, ::std::size_t N>
typename wavelet_tree<T, N>::size_type wavelet_tree<T, N>::select_at(size_type j, value_type c) const
{
    for (auto l = HEIGHT; l > 0; --l)
    {
        auto &bits = level_bits(l - 1);
        auto b = (c >> (l - 1)) & 1;
        j -= b ? num_zeros(l - 1) : 0;
        j = bits.select(j, b);
    }

    return j;
}

} // namespace internal

} // namespace desa

#endif // DESA_INTERNAL_WAVELET_TREE_HPP_
