#include <iostream>
#include <cstring>
#include "hash.h"
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
#define inf 2147483647


template <typename ID_TYPE>
class Filter_M1
{
public:
    Filter_M1() {}
    Filter_M1(int memory_KB)
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
    int max_value = 500;
};

template <typename ID_TYPE>
class Cell_M1
{
public:
    ID_TYPE key;
    int freq;
    int vote;
    int n_entries;
    int acc;
    int theta;
    int ori_n_entries;
    pair<int, int> *entry;

    int basic_bits;
    int *bits_of_entries;

    Cell_M1() {}

    Cell_M1(int N_ENTRY, int THETA, int BASIC_ENTRY_SIZE)
    {
        key = 0, freq = 0, vote = 0, acc = 0, ori_n_entries = N_ENTRY, n_entries = N_ENTRY, theta = THETA, basic_bits = BASIC_ENTRY_SIZE;
        entry = new pair<int, int>[n_entries];
        bits_of_entries = new int[n_entries];
        for (int i = 0; i < n_entries; i++)
        {
            bits_of_entries[i] = basic_bits;
        }
    }


    inline void entry_insert(int VV, int f) 
    {
        for (int i = 0; i < n_entries; i++) 
        {
            if (entry[i].first == VV)
            {
                if (i != 0)
                {
                    if (entry[i].second + 1 > entry[i - 1].second)
                    {
                        swap(entry[i], entry[i - 1]);
                        i--;
                    }
                }


                if (i == 0)
                {
                    if (entry[i].second + 1 > pow(2, bits_of_entries[i]))
                    {
                        n_entries--; 
                        bits_of_entries[i] += (16 + bits_of_entries[n_entries]);
                    }
                    entry[i].second++;
                    return;
                }
                else if (i == n_entries - 1)
                {
                    if (entry[i].second + 1 > pow(2, bits_of_entries[i]))
                    { 
                        if ((pow(2, bits_of_entries[0]) / entry[0].second) >= 2) 
                        { 
                            bits_of_entries[0]--;
                            bits_of_entries[i]++;
                        }
                        else
                        {
                            return;
                        }
                    }
                    entry[i].second++;
                    return;
                }
                else
                {
                    if (entry[i].second + 1 > pow(2, bits_of_entries[i]))
                    {

                        if ((pow(2, bits_of_entries[0]) / entry[0].second) >= 2)
                        { 
                            bits_of_entries[0]--;
                            bits_of_entries[i]++;

                        }
                        else
                        {

                            return;
                        }
                    }
                    entry[i].second++;
                }
                return;
            }

            if (entry[i].first == 0)
            {
                entry[i].first = VV;
                entry[i].second = 1;
                return; 
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
        n_entries = ori_n_entries;
        for (int i = 0; i < n_entries; i++)
        {
            bits_of_entries[i] = basic_bits;
            entry[i].first = 0;
            entry[i].second = 0;
        }
    }

    void show_info()
    {   

    }
};

template <typename ID_TYPE>
class Bucket_M1
{
public:
    int n_cells;
    int lambda;
    int n_entries;
    int theta;
    int basic_entry_size;
    Cell_M1<ID_TYPE> *cellgroup;

public:
    Bucket_M1() {}
    Bucket_M1(int N_CELLS, int LAMBDA, int N_ENTRIES, int THETA, int BASIC_ENTRY_SIZE)
    {

        n_cells = N_CELLS, n_entries = N_ENTRIES, lambda = LAMBDA, theta = THETA, basic_entry_size = BASIC_ENTRY_SIZE;
        cellgroup = new Cell_M1<ID_TYPE>[n_cells];
        for (int i = 0; i < n_cells; i++)
        {
            cellgroup[i] = Cell_M1<ID_TYPE>(n_entries, theta, basic_entry_size);
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
class PIE_M1
{
public:
    int n_cells;
    int n_entries;
    int lambda;
    int theta;
    int n_buckets;
    int basic_entry_size;
    int filter_thres;
    Bucket_M1<ID_TYPE> *bucketgroup;
    Filter_M1<ID_TYPE> *filter;
    bool ffg;
    PIE_M1() {}

    PIE_M1(int memory_KB, double FILTER_RATIO, int N_CELLS, int N_ENTRIES, int LAM, int THE, int THRES, int BASIC_ENTRY_SIZE)
    {
        n_cells = N_CELLS;
        n_entries = N_ENTRIES;
        lambda = LAM;
        theta = THE;
        filter_thres = THRES;
        basic_entry_size = BASIC_ENTRY_SIZE;
        ffg = true;
        n_buckets = int(memory_KB * (1 - FILTER_RATIO)) * 1024 * 8 / (n_cells * (8 + 24 + 16 + 16 + n_entries * (16 + BASIC_ENTRY_SIZE)));

        filter = new Filter_M1<ID_TYPE>(int((FILTER_RATIO)*memory_KB)); 
        bucketgroup = new Bucket_M1<ID_TYPE>[n_buckets];

        for (int i = 0; i < n_buckets; i++)
        {
            bucketgroup[i] = Bucket_M1<ID_TYPE>(n_cells, lambda, n_entries, theta, basic_entry_size);
        }
    }

    PIE_M1(int memory_KB, double FILTER_RATIO, int N_CELLS, int N_ENTRIES, int LAM, int THE, int THRES, int BASIC_ENTRY_SIZE, bool fff)
    {
        n_cells = N_CELLS;
        n_entries = N_ENTRIES;
        lambda = LAM;
        theta = THE;
        basic_entry_size = BASIC_ENTRY_SIZE;
        ffg = fff;
        n_buckets = (int)((memory_KB * 1024 * 8) / (n_cells * (8 + 24 + 16 + 16 + n_entries * (16 + BASIC_ENTRY_SIZE)))); 
        bucketgroup = new Bucket_M1<ID_TYPE>[n_buckets];
        for (int i = 0; i < n_buckets; i++)
        {
            bucketgroup[i] = Bucket_M1<ID_TYPE>(n_cells, lambda, n_entries, theta, basic_entry_size);
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
