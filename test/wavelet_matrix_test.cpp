/************************************************
 *  wavelet_matrix_test.cpp
 *  DICT
 *
 *  Copyright (c) 2015-2017, Chi-En Wu
 *  Distributed under The BSD 3-Clause License
 ************************************************/

#include <gtest/gtest.h>

#include <dict/internal/wavelet_matrix.hpp>

using wm_t = dict::internal::wavelet_matrix<char>;

// NOLINTNEXTLINE(runtime/references)
void construct_wavelet_matrix(wm_t &wt) {
    wt.insert(0, 'i');  // [i]
    wt.insert(0, 'i');  // [i]  i
    wt.insert(1, 's');  //  i  [s]  i
    wt.insert(3, 'p');  //  i   s   i  [p]
    wt.insert(2, 'i');  //  i   s  [i]  i   p
    wt.insert(3, 's');  //  i   s   i  [s]  i   p
    wt.insert(3, 's');  //  i   s   i  [s]  s   i   p
    wt.insert(0, 'm');  // [m]  i   s   i   s   s   i   p
    wt.insert(2, 's');  //  m   i  [s]  s   i   s   s   i   p
    wt.insert(9, 'i');  //  m   i   s   s   i   s   s   i   p  [i]
    wt.insert(9, 'p');  //  m   i   s   s   i   s   s   i   p  [p]  i
}

// NOLINTNEXTLINE(runtime/references)
void construct_wavelet_matrix_with_erasure(wm_t &wt) {
    construct_wavelet_matrix(wt);
    wt.erase(3);  // m i s i s s i p p i
    wt.erase(9);  // m i s i s s i p p
    wt.erase(0);  // i s i s s i p p
    wt.erase(5);  // i s i s s p p
    wt.erase(3);  // i s i s p p
}

TEST(WaveletTreeTest, EmptyTree) {
    wm_t wt;
    EXPECT_EQ(0, wt.size());
}

TEST(WaveletTreeTest, InsertFirstChar) {
    wm_t wt;
    wt.insert(0, 'X');
    EXPECT_EQ(1, wt.size());
}

TEST(WaveletTreeTest, EraseLastChar) {
    wm_t wt;
    wt.insert(0, 'X');
    auto c = wt.erase(0);
    EXPECT_EQ(0, wt.size());
    EXPECT_EQ('X', c);
}

TEST(WaveletTreeTest, InsertMoreChars) {
    wm_t wt;
    construct_wavelet_matrix(wt);
    EXPECT_EQ(11, wt.size());
}

TEST(WaveletTreeTest, EraseMoreChars) {
    wm_t wt;
    construct_wavelet_matrix_with_erasure(wt);
    EXPECT_EQ(6, wt.size());
}

TEST(WaveletTreeTest, AccessAfterInsertion) {
    wm_t wt;
    construct_wavelet_matrix(wt);
    EXPECT_EQ('m', wt[0]);
    EXPECT_EQ('i', wt[1]);
    EXPECT_EQ('s', wt[2]);
    EXPECT_EQ('s', wt[3]);
    EXPECT_EQ('i', wt[4]);
    EXPECT_EQ('s', wt[5]);
    EXPECT_EQ('s', wt[6]);
    EXPECT_EQ('i', wt[7]);
    EXPECT_EQ('p', wt[8]);
    EXPECT_EQ('p', wt[9]);
    EXPECT_EQ('i', wt[10]);
}

TEST(WaveletTreeTest, AccessAfterErasure) {
    wm_t wt;
    construct_wavelet_matrix_with_erasure(wt);
    EXPECT_EQ('i', wt[0]);
    EXPECT_EQ('s', wt[1]);
    EXPECT_EQ('i', wt[2]);
    EXPECT_EQ('s', wt[3]);
    EXPECT_EQ('p', wt[4]);
    EXPECT_EQ('p', wt[5]);
}

TEST(WaveletTreeTest, RankChars) {
    wm_t wt;
    construct_wavelet_matrix(wt);

    EXPECT_EQ(0, wt.rank(0, 'i'));
    EXPECT_EQ(1, wt.rank(1, 'i'));
    EXPECT_EQ(1, wt.rank(2, 'i'));
    EXPECT_EQ(1, wt.rank(3, 'i'));
    EXPECT_EQ(2, wt.rank(4, 'i'));
    EXPECT_EQ(2, wt.rank(5, 'i'));
    EXPECT_EQ(2, wt.rank(6, 'i'));
    EXPECT_EQ(3, wt.rank(7, 'i'));
    EXPECT_EQ(3, wt.rank(8, 'i'));
    EXPECT_EQ(3, wt.rank(9, 'i'));
    EXPECT_EQ(4, wt.rank(10, 'i'));

    EXPECT_EQ(0, wt.rank(0, 's'));
    EXPECT_EQ(0, wt.rank(1, 's'));
    EXPECT_EQ(1, wt.rank(2, 's'));
    EXPECT_EQ(2, wt.rank(3, 's'));
    EXPECT_EQ(2, wt.rank(4, 's'));
    EXPECT_EQ(3, wt.rank(5, 's'));
    EXPECT_EQ(4, wt.rank(6, 's'));
    EXPECT_EQ(4, wt.rank(7, 's'));
    EXPECT_EQ(4, wt.rank(8, 's'));
    EXPECT_EQ(4, wt.rank(9, 's'));
    EXPECT_EQ(4, wt.rank(10, 's'));
}

TEST(WaveletTreeTest, RankAndAccessChars) {
    wm_t wt;
    construct_wavelet_matrix(wt);

    using cr_pair = decltype(wt.access_and_rank(0));
    EXPECT_EQ(cr_pair('m', 1), wt.access_and_rank(0));
    EXPECT_EQ(cr_pair('i', 1), wt.access_and_rank(1));
    EXPECT_EQ(cr_pair('s', 1), wt.access_and_rank(2));
    EXPECT_EQ(cr_pair('s', 2), wt.access_and_rank(3));
    EXPECT_EQ(cr_pair('i', 2), wt.access_and_rank(4));
    EXPECT_EQ(cr_pair('s', 3), wt.access_and_rank(5));
    EXPECT_EQ(cr_pair('s', 4), wt.access_and_rank(6));
    EXPECT_EQ(cr_pair('i', 3), wt.access_and_rank(7));
    EXPECT_EQ(cr_pair('p', 1), wt.access_and_rank(8));
    EXPECT_EQ(cr_pair('p', 2), wt.access_and_rank(9));
    EXPECT_EQ(cr_pair('i', 4), wt.access_and_rank(10));
}

TEST(WaveletTreeTest, SelectChars) {
    wm_t wt;
    construct_wavelet_matrix(wt);

    EXPECT_EQ(1, wt.select(0, 'i'));
    EXPECT_EQ(4, wt.select(1, 'i'));
    EXPECT_EQ(7, wt.select(2, 'i'));
    EXPECT_EQ(10, wt.select(3, 'i'));

    EXPECT_EQ(2, wt.select(0, 's'));
    EXPECT_EQ(3, wt.select(1, 's'));
    EXPECT_EQ(5, wt.select(2, 's'));
    EXPECT_EQ(6, wt.select(3, 's'));
}

TEST(WaveletTreeTest, Sum) {
    wm_t wt;
    construct_wavelet_matrix(wt);
    EXPECT_EQ(0, wt.sum('i'));
    EXPECT_EQ(4, wt.sum('m'));
    EXPECT_EQ(5, wt.sum('p'));
    EXPECT_EQ(7, wt.sum('s'));
    EXPECT_EQ(11, wt.sum('t'));
}
