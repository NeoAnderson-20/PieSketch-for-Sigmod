#include <iostream>

using namespace std;

#define COUNTER_SIZE 32
#define KEY_LEN 4
#define inf 2147483647

template <typename ID_TYPE>
class CMSKETCH
{
public:
    CMSKETCH() {}
    CMSKETCH(int memory_KB)
    {   
        w = memory_KB * 1024 * 8 / 8 / d; // 这里每个counter 只要设置8位！
        array = new int *[d];
        for (int i = 0; i < d; i++)
        {
            array[i] = new int[w];
            memset(array[i], 0, w * sizeof(int));
        }
    }

    void insert(ID_TYPE id, int f = 1) // 返回查询结果
    {
        // cout << id<<" "<<f << endl;

        uint32_t index;
        for (int i = 0; i < d; i++)
        {
            // cout << i << endl;
            index = mhash(id, i * 100) % w;
            array[i][index] += f;
        }
    }
    int query(ID_TYPE id) // 返回查询结果
    {
        int result = inf;
        uint32_t index;

        for (int i = 0; i < d; i++)
        {
            index = mhash(id, i * 100) % w;
            array[i][index]++;
            result = MIN(result, array[i][index]);
        }
        return result;
    }

public:
    int d=3;
    int **array;
    int w;
    int max_value = 64;
};