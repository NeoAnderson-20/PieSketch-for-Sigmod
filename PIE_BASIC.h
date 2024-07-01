#include <iostream>
#include <cstring>
#include "hash.h"
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
#define inf 2147483647


template <typename ID_TYPE>
class Filter_BASIC
{
public:
    Filter_BASIC() {}
    Filter_BASIC(int memory_KB)
    {
        d = 3;
        w = memory_KB * 1024 * 8 / 8 / d; 
        array = new int *[d];
        for (int i = 0; i < d; i++)
        {
            array[i] = new int[w];
            for (int j = 0; j < w; j++)
            {
                array[i][j] = 0;
            }
        }
    }

    inline int insert(ID_TYPE id) 
    {
        int result = inf;

        for (int i = 0; i < d; i++)
        {
            uint32_t index = mhash(id, i * 100) % w;

            if (array[i][index] + 1 < max_value) 
            {
                array[i][index]++;
            }
            result = min(result, array[i][index]);
        }
        return result;
    }

public:
    int d;
    int **array;
    int w;
    int max_value = 50000;
};

template <typename ID_TYPE>
class Cell_BASIC
{
public:
    ID_TYPE key;
    int freq;
    int vote;
    int n_entries;
    int acc;
    int theta;
    pair<int, int> *entry;

    Cell_BASIC() {}

    Cell_BASIC(int N_ENTRY, int THETA)
    {
        key = 0, freq = 0, vote = 0, acc = 0, n_entries = N_ENTRY, theta = THETA;
        entry = new pair<int, int>[n_entries];
    }

    inline void entry_insert(int VV, int f)
    {
        for (int i = 0; i < n_entries; i++)
        {
            if (entry[i].first == VV)
            {                                     
                if (entry[i].second < pow(2, 16)) 
                {
                    entry[i].second++;
                }
                else
                {

                    return;
                }

                if (i != 0)
                {
                    if (entry[i].second > entry[i - 1].second)
                    {
                        swap(entry[i], entry[i - 1]);
                    }                                
                }

                return; 
            }
            if (entry[i].first == 0)
            {
                entry[i].first = VV;
                entry[i].second = 1;
                return;
            }
        }

        acc++;
        if (acc / entry[n_entries - 1].second > theta)
        {
            entry[n_entries - 1].first = VV;
            entry[n_entries - 1].second = 1;
            acc = 0; 
            return;
        }
        else
        {
            return;
        }
    }

    inline int point_query(int VV)
    {
        for (int i = 0; i < n_entries; i++)
        {
            if (entry[i].first == VV)
            {
                return entry[i].second;
            }
        }

        return 0;
    }

    inline vector<pair<int, int>> pie_query() 
    {
        vector<pair<int, int>> res;
        for (int i = 0; i < n_entries; i++)
        {
            if (entry[i].first == 0)
            {
                break;
            }
            res.push_back(entry[i]);
        }

        return res;
    }

    void clear_all()
    { 
        key = 0, freq = 0, vote = 0, acc = 0;
        for (int i = 0; i < n_entries; i++)
        {
            entry[i].first = 0;
            entry[i].second = 0;
        }
    }

    void show_info()
    {

    }
};

template <typename ID_TYPE>
class Bucket_BAISIC
{
public:
    int n_cells;
    int lambda;
    int n_entries;
    int theta;
    Cell_BASIC<ID_TYPE> *cellgroup;

public:
    Bucket_BAISIC() {}
    Bucket_BAISIC(int N_CELLS, int LAMBDA, int N_ENTRIES, int THETA)
    {
        n_cells = N_CELLS, n_entries = N_ENTRIES, lambda = LAMBDA, theta = THETA;
        cellgroup = new Cell_BASIC<ID_TYPE>[n_cells];
        for (int i = 0; i < n_cells; i++)
        {
            cellgroup[i] = Cell_BASIC<ID_TYPE>(n_entries, theta);
        }
    }

    inline void insert(ID_TYPE id, int value)
    {
        int minpos = -1;
        int minfreq = inf;

        for (int i = 0; i < n_cells; i++)
        {
            if (cellgroup[i].key == id)
            {
                cellgroup[i].freq++;
                cellgroup[i].entry_insert(value, 1);
                return;
            }

            if (cellgroup[i].key == 0)
            {
                cellgroup[i].key = id;
                cellgroup[i].freq = 1;
                cellgroup[i].entry_insert(value, 1);
                return;
            }

            if (cellgroup[i].freq < minfreq)
            { 
                minpos = i;
                minfreq = cellgroup[i].freq;
            }
        }

        cellgroup[minpos].vote++;
        if (cellgroup[minpos].vote / cellgroup[minpos].freq > lambda)
        {
            cellgroup[minpos].clear_all();
            cellgroup[minpos].key = id;
            cellgroup[minpos].freq = 1;
            cellgroup[minpos].entry_insert(value, 1);
        }
        return;
    }

    inline int point_query(ID_TYPE id, int VV)
    {

        for (int i = 0; i < n_cells; i++)
        {
            if (cellgroup[i].key == id)
            {

                return cellgroup[i].point_query(VV);
            }
        }
        return 0;
    }

    inline vector<pair<int, int>> pie_query(ID_TYPE id)
    {
        for (int i = 0; i < n_cells; i++)
        {
            if (cellgroup[i].key == id)
            {

                return cellgroup[i].pie_query();
            }
        }
        return vector<pair<int, int>>();
    }

    void show_info()
    {

    }
};

template <typename ID_TYPE>
class PIE_BAISIC
{
public:
    int n_cells;
    int n_entries;
    int lambda;
    int theta;
    int n_buckets;
    int filter_thres;
    bool ffg;
    Bucket_BAISIC<ID_TYPE> *bucketgroup;
    Filter_BASIC<ID_TYPE> *filter;

    PIE_BAISIC() {}

    PIE_BAISIC(int memory_KB, double FILTER_RATIO, int N_CELLS, int N_ENTRIES, int LAM, int THE, int THRES)
    {
        n_cells = N_CELLS;
        n_entries = N_ENTRIES;
        lambda = LAM;
        theta = THE;
        filter_thres = THRES;
        ffg = true;
        n_buckets = int(memory_KB * (1 - FILTER_RATIO)) * 1024 * 8 / (n_cells * (8 + 24 + 16 + 16 + n_entries * (16 + 16))); 
        filter = new Filter_BASIC<ID_TYPE>(int((FILTER_RATIO)*memory_KB));                                                   
        bucketgroup = new Bucket_BAISIC<ID_TYPE>[n_buckets];

        for (int i = 0; i < n_buckets; i++)
        {
            bucketgroup[i] = Bucket_BAISIC<ID_TYPE>(n_cells, lambda, n_entries, theta);
        }
    }

    PIE_BAISIC(int memory_KB, double FILTER_RATIO, int N_CELLS, int N_ENTRIES, int LAM, int THE, int THRES, bool whether_filter)
    {
        n_cells = N_CELLS;
        n_entries = N_ENTRIES;
        lambda = LAM;
        theta = THE;
        ffg = whether_filter;

        n_buckets = (memory_KB * 1024 * 8) / (n_cells * (8 + 24 + 16 + 16 + n_entries * (16 + 16))); 
        bucketgroup = new Bucket_BAISIC<ID_TYPE>[n_buckets];
        for (int i = 0; i < n_buckets; i++)
        {
            bucketgroup[i] = Bucket_BAISIC<ID_TYPE>(n_cells, lambda, n_entries, theta);
        }
    }

    inline void insert(ID_TYPE id, int value)
    {

        if (ffg)
        {
            if (filter->insert(id) < filter_thres)
            {
                return;
            }
        }

        uint32_t index = mhash(id, 100) % n_buckets; 
        bucketgroup[index].insert(id, value);
    }

    inline int point_query(ID_TYPE id, int value)
    {
        uint32_t index = mhash(id, 100) % n_buckets; 
        return bucketgroup[index].point_query(id, value);
    }

    inline vector<pair<int, int>> pie_query(ID_TYPE id)
    {
        uint32_t index = mhash(id, 100) % n_buckets;
        return bucketgroup[index].pie_query(id);
    }

    void show_info()
    {

    }
};
