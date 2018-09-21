// Copyright 2018 Chinaso Inc. All Rights Reserved.
// Author: zhankeyu

#include <string>
#include "basic_bloom_filter.h"
#include "counting_bloom_filter.h"
#include "partial_bloom_filter.h"
#include "murmur_hasher.h"
#include "gtest/include/gtest/gtest.h"

using std::string;
using bloom::BasicBloomFilter;
using bloom::Murmur2Hasher64A;
using bloom::CountingBloomFilter;
using bloom::PartialBloomFilter;

TEST(BloomFilter, BasicBloomFilter) {
 typedef BasicBloomFilter<Murmur2Hasher64A> BloomFilterType;

 BloomFilterType bf(1000000, 0.01);

 string cases[] = {
  "aaaa",
  "bbbb",
  "cccc",
  "dddd",
 };
 int cases_length = sizeof(cases)/sizeof(string);
 for (int i = 0; i < cases_length; ++i) {
  bf.Insert(cases[i]);
 }

 for (int i = 0; i < cases_length; ++i) {
  CHECK(bf.MayContain(cases[i]));
 }

 CHECK_EQ(bf.GetElementCount(), cases_length);
}

TEST(BloomFilter, CountingBloomFilter) {
 typedef CountingBloomFilter<Murmur2Hasher64A> BloomFilterType;

 BloomFilterType bf(1000000, 0.01);

 string cases[] = {
  "aaaa",
  "bbbb",
  "cccc",
  "dddd",
 };
 int cases_length = sizeof(cases)/sizeof(string);
 for (int i = 0; i < cases_length; ++i) {
  bf.Insert(cases[i]);
 }

 for (int i = 0; i < cases_length; ++i) {
  CHECK(bf.MayContain(cases[i]));
 }

 CHECK_EQ(bf.GetElementCount(), cases_length);

 for (int i = 0; i < cases_length; ++i) {
  bf.Delete(cases[i]);
 }

 for (int i = 0; i < cases_length; ++i) {
  CHECK(!bf.MayContain(cases[i]));
 }

 CHECK_EQ(bf.GetElementCount(), 0);
}

TEST(BloomFilter, PartialBloomFilter) {
 typedef PartialBloomFilter<Murmur2Hasher64A> BloomFilterType;

 BloomFilterType bf(1000001, 0.01);

 string cases[] = {
  "aaaa",
  "bbbb",
  "cccc",
  "dddd",
 };
 int cases_length = sizeof(cases)/sizeof(string);
 for (int i = 0; i < cases_length; ++i) {
  bf.Insert(cases[i]);
 }

 for (int i = 0; i < cases_length; ++i) {
  CHECK(bf.MayContain(cases[i]));
 }

 CHECK_EQ(bf.GetElementCount(), cases_length);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
