#include <iostream>
#include <cstring>
#include "hash.h"
#include <iostream>
#include <vector>
#include <utility>
#include <x86intrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <algorithm>

using namespace std;
#define inf 2147483647

struct Bucket_SIMD
{
    uint32_t keys[8];
    uint32_t freqs[8];
    uint32_t neg[8];
    uint32_t acc[8];
    uint32_t entry_k[8][8];
    uint32_t entry_v[8][8];
    int lambda = 3;
    int theta = 2;

    inline uint32_t Match_Key(const uint32_t item_key)
    {
        __m256i vec = _mm256_set1_epi32(item_key); //! 这里去256/32 ， 试试512/32
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i *)keys));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline uint32_t Match_Value(const uint32_t item_value, const uint32_t cell_pos)
    {
        __m256i vec = _mm256_set1_epi32(item_value);
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i *)entry_k[cell_pos]));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline void insert(const uint32_t item_key, const uint32_t item_value)
    {
        uint32_t match_key = Match_Key(item_key);
        if (match_key != 0)
        { // 匹配到了key

            uint32_t idx_of_key = __builtin_ctz(match_key); // 获得索引 __builtin_ctz!!!
            freqs[idx_of_key]++;

            uint32_t match_value = Match_Value(item_value, idx_of_key);
            if (match_value != 0) // 匹配到了value
            {
                uint32_t idx_of_value = __builtin_ctz(match_value);
                entry_v[idx_of_key][idx_of_value]++;

                // 排序
                if (idx_of_value > 0)
                {
                    if (entry_v[idx_of_key][idx_of_value] > entry_v[idx_of_key][idx_of_value - 1])
                    {
                        swap(entry_v[idx_of_key][idx_of_value], entry_v[idx_of_key][idx_of_value - 1]);
                        swap(entry_k[idx_of_key][idx_of_value], entry_k[idx_of_key][idx_of_value - 1]);
                    }
                }
            }
            else // 没匹配到value
            {
                match_value = Match_Value(0, idx_of_key); // 匹配value = 0
                if (match_value != 0)
                { // 如果匹配到了 0
                    uint32_t idx_of_value = __builtin_ctz(match_value);
                    entry_k[idx_of_key][idx_of_value] = item_value;
                    entry_v[idx_of_key][idx_of_value] = 1;
                }
                else
                { // 无空位！！！
                    acc[idx_of_key]++;
                    if (acc[idx_of_key] / entry_v[idx_of_key][7] > theta)
                    {
                        entry_k[idx_of_key][7] = item_value;
                        entry_v[idx_of_key][7] = 1;
                    }
                }
            }
        }
        else
        {
            match_key = Match_Key(0); // 匹配 key = 0;
            if (match_key != 0)
            {
                uint32_t idx_of_key = __builtin_ctz(match_key);
                keys[idx_of_key] = item_key;
                freqs[idx_of_key] = 1;
                entry_k[idx_of_key][0] = item_value;
                entry_v[idx_of_key][0] = 1;
            }
            else
            { // 替换最小的
                auto min_it = min_element(begin(freqs), end(freqs));
                int min_index = distance(begin(freqs), min_it);
                neg[min_index]++;
                if (neg[min_index] / freqs[min_index] > lambda)
                {
                    keys[min_index] = item_key;
                    freqs[min_index] = 1;
                    neg[min_index] = 0;
                    acc[min_index] = 0;
                    entry_k[min_index][0] = item_value;
                    entry_v[min_index][0] = 1;
                }
            }
        }
    }

    inline uint32_t point_query(const uint32_t item_key, const uint32_t item_value)
    {
        uint32_t match_key = Match_Key(item_key);
        if (match_key != 0)
        {                                                   // 匹配到了key
            uint32_t idx_of_key = __builtin_ctz(match_key); // 获得索引
            uint32_t match_value = Match_Value(item_value, idx_of_key);
            if (match_value != 0)
            {
                uint32_t idx_of_value = __builtin_ctz(match_value);
                return entry_v[idx_of_key][idx_of_value];
            }
        }

        return 0;
    }
};

class Pie_SIMD
{
public:
    int num_of_bkts;
    Bucket_SIMD *bkts;

    Pie_SIMD() {}
    Pie_SIMD(int memory_KB)
    {
        num_of_bkts = memory_KB * 1024 * 8 / (8 * (8 + 24 + 16 + 16 + 8 * (16 + 16)));
        bkts = new Bucket_SIMD[num_of_bkts];
    }

    inline void insert(const uint32_t key, const uint32_t value)
    {
        int idx_of_bkt = mhash(key, 100) % num_of_bkts;
        bkts[idx_of_bkt].insert(key, value);
    }

    inline int point_query(const uint32_t key, const uint32_t value)
    {
        int idx_of_bkt = mhash(key, 100) % num_of_bkts;
        return bkts[idx_of_bkt].point_query(key, value);
    }
};
