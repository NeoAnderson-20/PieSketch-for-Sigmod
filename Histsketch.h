#include <iostream>
#include "CM.h"
#include "BOBHash.h"
#include "hash.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>

using namespace std;

template <typename ID_TYPE>
class Hist_Bucket
{
public:
    ID_TYPE K;
    int N;
    pair<int, int> *layer_1;
    pair<int, int> *layer_2;
    int n_slots;

public:
    Hist_Bucket() {}

    Hist_Bucket(int N_SLOTS)
    {
        n_slots = N_SLOTS;
        K = 0;
        N = 0;
        layer_1 = new pair<int, int>[n_slots];
        layer_2 = new pair<int, int>[n_slots];

        for (int i = 0; i < n_slots; i++)
        {
            layer_1[i] = make_pair(0, 0);
            layer_2[i] = make_pair(0, 0);
        }
    }

    pair<int, int> insert_slot(int value, int f = 1)
    {
        int index1 = mhash(value, 100) % n_slots;
        if (layer_1[index1].first == value)
        {
            layer_1[index1].second++;
            return make_pair(0, 0);
        }

        if (layer_1[index1].first == 0)
        {
            layer_1[index1].first = value;
            layer_1[index1].second = 1;
            return make_pair(0, 0);
        }

        pair<int, int> l1_evicted = layer_1[index1];
        layer_1[index1].first = value;
        layer_1[index1].second = 1;

        int index2 = mhash(l1_evicted.first, 200) % n_slots;
        if (layer_2[index2].first == l1_evicted.first)
        {
            layer_2[index2].second += l1_evicted.second;
            return make_pair(0, 0);
        }

        if (layer_2[index2].first == 0)
        {
            layer_2[index2].first = l1_evicted.first;
            layer_2[index2].second = l1_evicted.second;
            return make_pair(0, 0);
        }

        if (layer_2[index2].second >= l1_evicted.second)
        {
            return l1_evicted;
        }
        else
        {
            pair<int, int> l2_evicted = layer_2[index2];
            layer_2[index2].first = l1_evicted.first;
            layer_2[index2].second = l1_evicted.second;
            return l2_evicted;
        }
        return make_pair(0, 0);
    }

    int point_query(int value)
    {
        int index1 = mhash(value, 100) % n_slots;
        int index2 = mhash(value, 200) % n_slots;
        if (layer_1[index1].first == value)
        {
            return layer_1[index1].second;
        }
        if (layer_2[index2].first == value)
        {
            return layer_2[index2].second;
        }
        return 0;
    }

    vector<pair<int, int>> pie_query() // 排序一下,然后返回，作为pie估计结果
    {
        vector<pair<int, int>> res;
        for (int i = 0; i < n_slots; i++)
        {
            if (layer_1[i].first != 0)
            {
                res.push_back(layer_1[i]);
            }
        }

        for (int i = 0; i < n_slots; i++)
        {
            if (layer_2[i].first != 0)
            {

                res.push_back(layer_2[i]);
            }
        }
        sort(res.begin(), res.end(), compareSecond);
        return res;
    }

    int total_freqsum()
    {
        int fsum = 0;
        for (int i = 0; i < n_slots; i++)
        {
            fsum += layer_1[i].second;
            fsum += layer_2[i].second;
        }
        return fsum;
    }

    void clear_all()
    {
        K = 0;
        N = 0;
        for (int i = 0; i < n_slots; i++)
        {
            layer_1[i].first = 0;
            layer_1[i].second = 0;
            layer_2[i].first = 0;
            layer_2[i].second = 0;
        }
    }

    static bool compareSecond(const pair<int, int> &a, const pair<int, int> &b)
    {
        return a.second > b.second;
    }
};

template <typename ID_TYPE>
class Hist_01
{
public:
    int n_buckets;
    int n_slots;
    CMSKETCH<ID_TYPE> *cm_hist;
    Hist_Bucket<ID_TYPE> *buckets;

public:
    Hist_01() {}

    Hist_01(int memory_KB, int N_SLOTS, double CM_RATIO)
    {
        n_slots = N_SLOTS;
        n_buckets = memory_KB * (1 - CM_RATIO) * 1024 * 8 / (32 + 32 + 2 * n_slots * (16 + 16)); // 注意，hist有2层的n_slots！
        buckets = new Hist_Bucket<ID_TYPE>[n_buckets];
        for (int i = 0; i < n_buckets; i++)
        {
            buckets[i] = Hist_Bucket<ID_TYPE>(n_slots);
        }
        cm_hist = new CMSKETCH<ID_TYPE>((int)memory_KB * CM_RATIO);
    }

    void insert(ID_TYPE id, int value)
    {
        int bucket_index = mhash(id, 250) % n_buckets;
        if (buckets[bucket_index].K == id)
        {
            pair<int, int> rs = buckets[bucket_index].insert_slot(value);
            if (rs.first != 0)
            {
                string strA = to_string(id).substr(0, 8);
                string strB = to_string(rs.first).substr(0, 5);
                string strC = strA + strB;
                uint64_t c = stoll(strC); // 超出长度，需要截断！！！
                cm_hist->insert(c, rs.second);
            }
            else
            {
                string strA = to_string(id).substr(0, 8);
                string strB = to_string(value).substr(0, 5);
                string strC = strA + strB;
                uint64_t c = stoll(strC); // 超出长度，需要截断！！！
                cm_hist->insert(c, 1);
            }
            return;
        }
        // 务必确认有没有问题！
        if (buckets[bucket_index].K == 0)
        {
            buckets[bucket_index].K = id;
            buckets[bucket_index].insert_slot(value);
            return;
        }

        buckets[bucket_index].N++;
        int fsum = buckets[bucket_index].total_freqsum();
        if (buckets[bucket_index].N > fsum)
        {
            buckets[bucket_index].clear_all();
            buckets[bucket_index].K = id;
            buckets[bucket_index].insert_slot(value);
            return;
        }
    }

    int point_query(ID_TYPE id, int value)
    {
        int bucket_index = mhash(id, 250) % n_buckets;
        if (buckets[bucket_index].K == id)
        {
            int v = buckets[bucket_index].point_query(value);
            if (v == 0)
            {
                string strA = to_string(id).substr(0, 8);
                string strB = to_string(value).substr(0, 5);
                string strC = strA + strB;
                uint64_t c = stoll(strC);
                return cm_hist->query(c);
            }
            else
            {
                return v;
            }
        }
        return 0;
    }

    vector<pair<int, int>> pie_query(ID_TYPE id)
    {
        vector<pair<int, int>> res;

        int bucket_index = mhash(id, 250) % n_buckets;
        if (buckets[bucket_index].K == id) // 匹配到bucket
        {
            return buckets[bucket_index].pie_query();
        }
        else
        {
            return res;
        }
    }
};