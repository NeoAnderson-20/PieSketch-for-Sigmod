#include <iostream>
#include <cstring>
#include "hash.h"
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
#define inf 2147483647

// ! Morphing 应该是cell内部的？

template <typename ID_TYPE>
class Filter_M1
{
public:
    Filter_M1() {}
    Filter_M1(int memory_KB)
    {
        d = 3;
        w = memory_KB * 1024 * 8 / 8 / d; // 这里每个counter 只要设置8位！
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

    inline int insert(ID_TYPE id) // 返回查询结果
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

    // * Morphing的补充参数：
    int basic_bits;
    int *bits_of_entries;
    // vector<string> morph_history;

    Cell_M1() {}

    // 出现overflow的应该是最大的？
    Cell_M1(int N_ENTRY, int THETA, int BASIC_ENTRY_SIZE)
    {
        key = 0, freq = 0, vote = 0, acc = 0, ori_n_entries = N_ENTRY, n_entries = N_ENTRY, theta = THETA, basic_bits = BASIC_ENTRY_SIZE;
        entry = new pair<int, int>[n_entries];
        bits_of_entries = new int[n_entries];
        for (int i = 0; i < n_entries; i++)
        {
            bits_of_entries[i] = basic_bits;
        }
        // * 每个entry的初始最大大小
    }

    // 只有当溢出时，才会采用变形；  最大的变形，牺牲最小的；
    // 第二大的变形，剥夺最大的，分配；最小的溢出， ===》 丢弃 or 更换更大容量的bucket？

    inline void entry_insert(int VV, int f) //!  溢出就自动变形； 先看看最小的溢出多少
    {
        for (int i = 0; i < n_entries; i++) // 先交换再溢出
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

                // 如果交换失败
                //  如果是头部ent

                if (i == 0)
                {
                    if (entry[i].second + 1 > pow(2, bits_of_entries[i]))
                    {
                        n_entries--; // 最后一个entry不再会被索引到，逻辑上被删除了
                        bits_of_entries[i] += (16 + bits_of_entries[n_entries]);
                        // morph_history.push_back("MorphingTop");
                    }
                    entry[i].second++;
                    return;
                }
                else if (i == n_entries - 1)
                {
                    if (entry[i].second + 1 > pow(2, bits_of_entries[i]))
                    { 
                        if ((pow(2, bits_of_entries[0]) / entry[0].second) >= 2)  //最大值，已使用值
                        { 
                            bits_of_entries[0]--;
                            bits_of_entries[i]++;
                            // morph_history.push_back("LBS");
                        }
                        else
                        {
                            // morph_history.push_back("LBF");
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
                        // cout << "MIDDLE OVF!" << endl;
                        // 中间溢出,从最高位借一bit
                        if ((pow(2, bits_of_entries[0]) / entry[0].second) >= 2)
                        { // 如果最大值是当前值的2倍，说明有多余的bit
                            bits_of_entries[0]--;
                            bits_of_entries[i]++;
                            // morph_history.push_back("MBS");
                            // cout << "BORROW SUCCESS!" << endl;
                        }
                        else
                        {
                            // morph_history.push_back("MBF");
                            // cout << "BORROW FAILED!" << endl;
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
                return; // 放入第一个空位
            }
        }

        // 未匹配到，末尾替换
        acc++;
        if (acc / entry[n_entries - 1].second > theta)
        {
            entry[n_entries - 1].first = VV;
            entry[n_entries - 1].second = 1;
            acc = 0; // 替换一次后acc清空
            return;
        }
        else
        {
            return; // 返回
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

    inline vector<pair<int, int>> pie_query() // pie查询，就把所有的都返回出去; 其中第一个是key+freq
    {
        vector<pair<int, int>> res;
        // res.push_back(make_pair(key, freq));

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
    { // 清空
        // morph_history.push_back("NEW");
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
        cout<<"ERROR"<<endl;
        // ofstream ininfo("InternalInfo/LAT_MOR.txt", ios::app);
        // ininfo << "key=" << key << endl;
        // ininfo << "freq=" << freq << endl;
        // ininfo << "vote=" << vote << endl;
        // ininfo << "n_entries=" << n_entries << endl;
        // ininfo << "acc=" << acc << endl;
        // ininfo << "history = [ ";

        // for (auto i : morph_history)
        // {
        //     ininfo << " " << i;
        // }
        // ininfo << "]" << endl;

        // ininfo << "[";
        // for (int i = 0; i < n_entries; i++)
        // {
        //     ininfo << " " << bits_of_entries[i];
        // }
        // ininfo << "]" << endl;

        // for (int i = 0; i < n_entries; i++)
        // {
        //     ininfo << "< " << entry[i].first << " - " << entry[i].second << " >" << endl;
        // }
        // ininfo.close();
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
            { // 找出最小位置
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
        for (int i = 0; i < n_cells; i++)
        {
            ofstream ininfo("InternalInfo/LAT_MOR.txt", ios::app);
            ininfo << "======================================" << endl;
            cellgroup[i].show_info();
            ininfo.close();
        }
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
        // 单个bucket的大小  n_cells *(key+freq+vote+acc+n_entries*(entrysize))      // 使用fp之后是8位
        n_buckets = int(memory_KB * (1 - FILTER_RATIO)) * 1024 * 8 / (n_cells * (8 + 24 + 16 + 16 + n_entries * (16 + BASIC_ENTRY_SIZE))); // 实际没这么大！！
        // cout << "mor_cells = " << n_buckets *n_cells<< endl;

        filter = new Filter_M1<ID_TYPE>(int((FILTER_RATIO)*memory_KB)); // 单个cell大小 =  32(key)+32(freq)+32(vote)+ 32(acc)+n_entries*(32+32))    默认全是32位！！！
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
        n_buckets = (int)((memory_KB * 1024 * 8) / (n_cells * (8 + 24 + 16 + 16 + n_entries * (16 + BASIC_ENTRY_SIZE)))); // 实际没这么大！！
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

        uint32_t index = mhash(id, 100) % n_buckets; // hash到一个bucket中
        bucketgroup[index].insert(id, value);
    }

    inline int point_query(ID_TYPE id, int value)
    {
        uint32_t index = mhash(id, 100) % n_buckets; // hash到一个bucket中
        return bucketgroup[index].point_query(id, value);
    }

    inline vector<pair<int, int>> pie_query(ID_TYPE id)
    {
        uint32_t index = mhash(id, 100) % n_buckets; // hash到一个bucket中
        return bucketgroup[index].pie_query(id);
    }

    void show_info()
    {
        ofstream ininfo("InternalInfo/LAT_MOR.txt", ios::out | ios::trunc);
        for (int i = 0; i < n_buckets; i++)
        {

            bucketgroup[i].show_info();
        }
        ininfo.close();
    }
};
