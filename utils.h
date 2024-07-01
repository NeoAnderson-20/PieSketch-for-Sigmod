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

void readTrace(const char *path, std::vector<Data> &dataVector)
{   

    ifstream inputFile(path);

    dataVector.clear(); 

    string line;
    while (getline(inputFile, line))
    {
        istringstream iss(line);
        Data data;

        if (line.find(':') != std::string::npos) 
        {
            continue;
        }

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
struct DataLat
{
    uint64_t src;
    int gap;
};
void readTraceLat(const char *path, std::vector<DataLat> &dataVector)
{
    ifstream inputFile(path);

    dataVector.clear(); 

    string line;
    while (getline(inputFile, line))
    {
        istringstream iss(line);
        DataLat dataLat;

        if (line.find(':') != std::string::npos) 
        {
            continue;
        }

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
    dataVector.clear();
    string line;
    uint64_t k = 0;
    int v = 0;
    while (getline(inputFile, line))
    {   
        if(k==150000){break;}
        ++k;
        istringstream iss(line);
        if (line.find(':') != std::string::npos)
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

uint32_t getFP(uint64_t key)
{
    return BOBHash::BOBHash32((uint8_t *)&key, 8, 333) % 0xFFFF + 1;
}
