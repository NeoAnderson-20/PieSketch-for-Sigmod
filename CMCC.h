#include <iostream>
#include "BOBHash.h"
#include "hash.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
using namespace std;

#define COUNTER_SIZE 32
#define KEY_LEN 4
#define inf 2147483647

template <typename ID_TYPE>
class CMCC
{
public:
    int d;
    int w;
    int n_slots;
    vector<vector<vector<pair<int, int>>>> array;

public:
    CMCC() {}
    CMCC(int memory_KB, int N_SLOTS, int D)
    {
        n_slots = N_SLOTS;
        d = D;
        w = memory_KB * 1024 * 8 / (n_slots * (32 + 32) * d); 
        array.resize(d, vector<vector<pair<int, int>>>(w, vector<pair<int, int>>(n_slots, make_pair(0, 0))));

    }

    void insert(ID_TYPE id, int value, int f = 1) 
    {
        uint32_t index;
        int minpos = -1;
        int mincnt = inf;
        for (int i = 0; i < d; i++)
        {
            bool flg = false;
            index = mhash(id, (i + 1) * 100) % w;
            for (int j = 0; j < n_slots; j++)
            {

                if (array[i][index][j].first == value)
                {
                    array[i][index][j].second++;
                    flg = true;
                    break;
                }
                if (array[i][index][j].first == 0)
                {
                    array[i][index][j].first = value;
                    array[i][index][j].second = 1;
                    flg = true;
                    break;
                }

                if (array[i][index][j].second < mincnt)
                {
                    mincnt = array[i][index][j].second;
                    minpos = j;
                }
            }
            if (!flg)
            {
            

                if(array[i][index][minpos].second == 1){
                    array[i][index][minpos].first = value;

                }else{
                    array[i][index][minpos].second--;
                }

            }
        }
    }

    int point_query(ID_TYPE id, int value) 
    {
        uint32_t index;
        int res = inf;

        for (int i = 0; i < d; i++)
        {
            index = mhash(id, (i + 1) * 100) % w;
            for (int j = 0; j < n_slots; j++)
            {

                if (array[i][index][j].first == value)
                {
                    if (array[i][index][j].second < res)
                    {

                        res = array[i][index][j].second;
                    }
                    break;
                }
            }
        }

        if (res == inf)
        {
            return 0;
        }
        else
        {
            return res;
        }
    }

    vector<pair<int, int>> pie_query(ID_TYPE id)
    {

        uint32_t index;
        map<int, int> mp;
        vector<pair<int, int>> res;

        for (int i = 0; i < d; i++)
        {
            index = mhash(id, (i + 1) * 100) % w;

            for (auto it = array[i][index].begin(); it != array[i][index].end(); ++it)
            {
                if (mp[it->first] == 0)
                {
                    mp[it->first] = it->second;
                }
                if (mp[it->first] != 0)
                {
                    mp[it->first] = min(mp[it->first], it->second);
                }
            }
        }

        for (auto it = mp.begin(); it != mp.end(); ++it)
        {
            res.push_back(make_pair(it->first, it->second));
        }
        sort(res.begin(), res.end(), compareSecond);

        return res;
    }

    static bool compareSecond(const pair<int, int> &a, const pair<int, int> &b)
    {
        return a.second > b.second;
    }
};