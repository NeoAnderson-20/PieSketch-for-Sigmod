#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "BOBHash.h"
#include <cstring>
using namespace std;

struct Data
{
    uint64_t src;
    uint64_t dst;
    int packet_size;
};

// 函数用于读取txt文件中的数据并存储到vector中
void readTrace(const char *path, std::vector<Data> &dataVector)
{   

    ifstream inputFile(path);

    dataVector.clear(); // 清空向量

    string line;
    while (getline(inputFile, line))
    {
        istringstream iss(line);
        Data data;

        if (line.find(':') != std::string::npos) // 跳过ipv6
        {
            continue;
        }

        // 通过三个空格分隔字符串
        if (iss >> data.src >> data.dst >> data.packet_size)
        {
            dataVector.push_back(data);
        }
        else
        {
            cerr << "Error parsing line: " << line << endl;
        }
    }

    inputFile.close();
}

//  读取 延迟 数据集！！！ 包括seattle！！
struct DataLat
{
    uint64_t src;
    int gap;
};
void readTraceLat(const char *path, std::vector<DataLat> &dataVector)
{
    ifstream inputFile(path);

    dataVector.clear(); // 清空向量

    string line;
    while (getline(inputFile, line))
    {
        istringstream iss(line);
        DataLat dataLat;

        if (line.find(':') != std::string::npos) // 跳过ipv6
        {
            continue;
        }

        // 通过三个空格分隔字符串
        if (iss >> dataLat.src >> dataLat.gap)
        {
            dataVector.push_back(dataLat);
        }
        else
        {
            cerr << "Error parsing line: " << line << endl;
        }
    }
    inputFile.close();
}

void readTraceWebDocs(const char *path, std::vector<pair<uint64_t, int>> &dataVector)
{

    ifstream inputFile(path);
    dataVector.clear(); // 清空向量
    string line;
    uint64_t k = 0;
    int v = 0;
    while (getline(inputFile, line))
    {   
        if(k==150000){break;}
        ++k;
        istringstream iss(line);
        if (line.find(':') != std::string::npos) // 跳过ipv6
        {
            continue;
        }
        while(iss >> v)
        {  
            dataVector.push_back(make_pair(k, v));
        }
    }
    inputFile.close();
}

// void readTraceSeattle(const char *path, std::vector<pair<uint64_t,int>> &dataVector)
// {
//     ifstream inputFile(path);
//     dataVector.clear(); // 清空向量
//     string line;
//     while (getline(inputFile, line))
//     {
//         istringstream iss(line);
//         uint64_t id;
//         int gap;
//         if (line.find(':') != std::string::npos) // 跳过ipv6
//         {
//             continue;
//         }
//         // 通过2个空格分隔字符串
//         if (iss >> id >> gap)
//         {
//             dataVector.push_back(make_pair(id, gap));
//         }
//         else
//         {
//             cerr << "Error parsing line: " << line << endl;
//         }
//     }
//     inputFile.close();
// }

// 获取16位的fp
uint32_t getFP(uint64_t key)
{
    // return key;
    return BOBHash::BOBHash32((uint8_t *)&key, 8, 333) % 0xFFFF + 1;
}
