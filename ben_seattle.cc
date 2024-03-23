

#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <vector>
#include "utils.h"
#include <map>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <ctime>
#include "CMCC.h"
#include "Histsketch.h"
#include <sys/time.h>
#include <unistd.h>
#include "PIE_BASIC.h"
#include "PIE_MORPHING.h"
using namespace std;
#define RED_TEXT "\033[1;31m"
#define GREEN_TEXT "\033[1;32m"
#define BLUE_TEXT "\033[1;34m"
const char *path = "dataset/sea_lat.txt";
const char *res_path_cmp = "result/result_sea/SEA_CMP.csv";
const char *res_param_path = "result/result_sea/sea_para_02.csv";
const char *res_path_cmp_pie = "result/result_sea/sea_pie_02.csv";
int TOPNUMBER = 10;

vector<DataLat> traces;
map<uint64_t, unordered_map<int, int>> ground_truth_all;
map<uint64_t, map<int, int>> ground_truth_large;
map<uint64_t, int> ground_truth_size;
map<uint64_t, vector<pair<int, int>>> sorted_ground_truth;
uint32_t total_packet = 0;

bool comparePairs(const pair<int, int> &left, pair<int, int> &right)
{
    return left.second > right.second;
}

// * 计算，两个pie_chart的值误差
double calculateTotalDifference(const uint64_t k, const vector<pair<int, int>> &v1, const vector<pair<int, int>> &v2, double portion_thres)
{
    int real_sum = ground_truth_size[k];
    unordered_map<int, double> v2_map;
    for (const auto &pair : v2)
    {
        v2_map[pair.first] = pair.second;
    }

    unordered_map<int, double> v1_map;
    for (const auto &pair : v1)
    {
        v1_map[pair.first] = pair.second;
    }

    double totalDifference = 0;

    for (const auto &pair : v1)
    {

        if (ground_truth_size[k] != 0 && (double)pair.second / ground_truth_size[k] < portion_thres)
        {
            continue;
        }

        auto it = v2_map.find(pair.first);
        if (it != v2_map.end())
        {
            totalDifference += (double)abs(pair.second - it->second);
        }
        else
        {
            totalDifference += pair.second;
        }
    }

    return totalDifference / real_sum;
}

// * 计算，两个pie_chart的比例误差
double calculatePortionDifference(const uint64_t k, const vector<pair<int, int>> &v1, const vector<pair<int, int>> &v2, double portion_thres)
{
    double real_sum = ground_truth_size[k];
    double estimated_sum = 0;

    unordered_map<int, double> v2_map;
    for (const auto &pair : v2)
    {
        v2_map[pair.first] = pair.second;
        estimated_sum += pair.second;
    }

    unordered_map<int, double> v1_map;
    for (const auto &pair : v1)
    {
        v1_map[pair.first] = pair.second;
    }

    double totalDifference = 0;

    for (const auto &pair : v1)
    {
        if (ground_truth_size[k] != 0 && (double)pair.second / ground_truth_size[k] < portion_thres)
        {
            continue;
        }
        auto it = v2_map.find(pair.first);
        if (it != v2_map.end())
        {
            totalDifference += (double)abs((double)pair.second / real_sum - (double)it->second / estimated_sum);
        }
        else
        {
            totalDifference += (double)pair.second / real_sum;
        }
    }

    return totalDifference;
}

void Find_Best_Param(int MEM_RANGE[], int s1, int FILTER_THRES_RANGE[], int s2,
                     int THETA_RANGE[], int s3, int LAMBDA_RANGE[], int s4, int N_ENTRIES_RANGE[], int s5,
                     int N_CELLS_RANGE[], int s6, int MOR_SIZE_RANGE[], int s7, double FILTER_PERCENT_RANGE[], int s8,
                     int flow_thres, double portion_thres)
{
    vector<int> vf_mem;
    vector<double> vf_filter_thresh_range;
    vector<int> vf_theta_range;
    vector<int> vf_lambda_range;
    vector<int> vf_ncells_range;
    vector<int> vf_nentries_range;
    vector<int> vf_mor_size_range;
    vector<double> vf_filter_percent_range;
    vector<PIE_BAISIC<uint64_t> *> vf_pies;
    vector<PIE_M1<uint64_t> *> vf_mors;

    for (int i = 0; i < s1; i++)
    {
        for (int k = 0; k < s2; ++k)
        {
            for (int l = 0; l < s5; ++l)
            {
                for (int m = 0; m < s6; ++m)
                {
                    for (int z = 0; z < s4; ++z)
                    {
                        for (int x = 0; x < s3; ++x)
                        {

                            for (int o = 0; o < s7; ++o)
                            {
                                for (int p = 0; p < s8; ++p)
                                {
                                    int _mem = MEM_RANGE[i];
                                    int _filter_thres = FILTER_THRES_RANGE[k];
                                    int _n_entries = N_ENTRIES_RANGE[l];
                                    int _n_cells = N_CELLS_RANGE[m];
                                    int _lambda = LAMBDA_RANGE[z];
                                    int _theta = THETA_RANGE[x];
                                    int _mor_size = MOR_SIZE_RANGE[o];
                                    double _filter_percent = FILTER_PERCENT_RANGE[p];

                                    vf_pies.push_back(new PIE_BAISIC<uint64_t>(_mem, _filter_percent, _n_cells, _n_entries, _lambda, _theta, _filter_thres, false));
                                    vf_mors.push_back(new PIE_M1<uint64_t>(_mem, _filter_percent, _n_cells, _n_entries, _lambda, _theta, _filter_thres, _mor_size, false));

                                    vf_mem.push_back(_mem);
                                    vf_filter_thresh_range.push_back(_filter_thres);
                                    vf_theta_range.push_back(_theta);
                                    vf_lambda_range.push_back(_lambda);
                                    vf_ncells_range.push_back(_n_cells);
                                    vf_nentries_range.push_back(_n_entries);
                                    vf_mor_size_range.push_back(_mor_size);
                                    vf_filter_percent_range.push_back(_filter_percent);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    cout << "Total Sketches = " << vf_pies.size() << endl;
    // time_t now = time(nullptr);
    // tm *local_time = localtime(&now);
    // char time_str[20];
    // strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    readTraceLat(path, traces);

    total_packet = traces.size();
    cout << "total_packet=" << total_packet << endl;
    int ss = total_packet;
    cout << "TOTAL ITEMS =" << total_packet << endl;
    map<int, double> total_insert_times_pie;
    map<int, double> total_insert_times_mor;

    for (auto it = traces.begin(); it != traces.end(); it++)
    {
        ss--;
        if (ss == 0)
        {
            break;
        }
        uint64_t src = it->src;
        int gap = it->gap + 1;

        // uint16_t srcFp = getFP(src);    // fingerprint
        uint64_t srcFp = src;

        int turn = 0;

        for (auto sketch : vf_pies)
        {
            auto t_a = chrono::high_resolution_clock::now();
            sketch->insert(srcFp, gap);
            auto t_b = std::chrono::high_resolution_clock::now();
            total_insert_times_pie[turn] += chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count();
            turn++;
        }

        turn = 0;
        for (auto sketch : vf_mors)
        {
            auto t_a = chrono::high_resolution_clock::now();
            sketch->insert(srcFp, gap);
            auto t_b = std::chrono::high_resolution_clock::now();
            total_insert_times_mor[turn] += chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count();
            turn++;
        }

        ground_truth_all[src][gap]++;
        ground_truth_size[src]++;
    }

    for (auto outerPair : ground_truth_all)
    {
        vector<pair<int, int>> tempVector(outerPair.second.begin(), outerPair.second.end());
        sort(tempVector.begin(), tempVector.end(), comparePairs);
        sorted_ground_truth[outerPair.first] = tempVector;
    }

    // 至此，全部插入完成。 开始查询与写入文件

    map<int, double> are_pies;
    map<int, double> are_mors;
    map<int, double> query_times_pies;
    map<int, double> query_times_mors;
    int total_estimated_flows = 0;
    int total_estimated_items = 0;

    for (auto it = sorted_ground_truth.begin(); it != sorted_ground_truth.end(); ++it)
    {

        if (ground_truth_size[it->first] < flow_thres)
        {
            continue; // 跳过小于FLOW_THRESH的流
        }

        total_estimated_flows++;
        int topV = TOPNUMBER;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            if (topV == 0)
            {
                break;
            }
            topV--;
            if (ground_truth_size[it->first] != 0 && (double)it2->second / ground_truth_size[it->first] < portion_thres)
            {
                break; // 占比小于5%的直接滚蛋！！！
            }
            total_estimated_items++;
            int turn = 0;

            for (auto sketch : vf_pies)
            {
                auto t_a_2 = chrono::high_resolution_clock::now();
                int pie_res = sketch->point_query((it->first), it2->first);
                auto t_b_2 = std::chrono::high_resolution_clock::now();
                query_times_pies[turn] += chrono::duration_cast<std::chrono::microseconds>(t_b_2 - t_a_2).count();
                are_pies[turn] += abs((double)pie_res - (double)it2->second) / it2->second;
                turn++;
            }

            turn = 0;
            for (auto sketch : vf_mors)
            {
                auto t_a_3 = chrono::high_resolution_clock::now();
                int mor_res = sketch->point_query((it->first), it2->first);
                auto t_b_3 = std::chrono::high_resolution_clock::now();
                query_times_mors[turn] += chrono::duration_cast<std::chrono::microseconds>(t_b_3 - t_a_3).count();
                are_mors[turn] += abs((double)mor_res - (double)it2->second) / it2->second;
                turn++;
            }
        }
    }

    // 至此，查询完成，开始输出结果
    ofstream resfile(res_param_path, ios::app);
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&currentTime);

    // 输出本地时间的各个组成部分
    std::cout << "Formatted Local Time: ";
    std::cout << std::put_time(localTime, "%y-%m-%d %H:%M:%S") << std::endl;

    // resfile << std::put_time(localTime, "%y-%m-%d %H:%M:%S") << endl;
    // resfile << "FLOW_THRESH = " << flow_thres << ", PORTION_THRESH=" << portion_thres << endl;
    resfile << "FLOW_THRESH,PORTION_THRESH,MEMORY,Filter_THRESH,FILTER_PERCENT,N_CELLS,N_ENTRIES,LAMDBA,THETH,MOR_SIZE,PIE_ARE,MOR_ARE,PIE_INSERT_TPR,PIE_QUERY_TPR,MOR_INSERT_TPR,MOR_QUERY_TPR" << endl;

    reverse(vf_mem.begin(), vf_mem.end());
    reverse(vf_filter_thresh_range.begin(), vf_filter_thresh_range.end());
    reverse(vf_ncells_range.begin(), vf_ncells_range.end());
    reverse(vf_nentries_range.begin(), vf_nentries_range.end());
    reverse(vf_lambda_range.begin(), vf_lambda_range.end());
    reverse(vf_theta_range.begin(), vf_theta_range.end());
    reverse(vf_mor_size_range.begin(), vf_mor_size_range.end());
    reverse(vf_filter_percent_range.begin(), vf_filter_percent_range.end());
    int how_many_sketches = are_pies.size();

    for (int i = 0; i < how_many_sketches; i++)
    {

        int _mem = vf_mem.back();
        int _filter_thres = vf_filter_thresh_range.back();
        int _n_entries = vf_nentries_range.back();
        int _n_cells = vf_ncells_range.back();
        int _lambda = vf_lambda_range.back();
        int _theta = vf_theta_range.back();
        int _mor_size = vf_mor_size_range.back();
        double _filter_percent = vf_filter_percent_range.back();

        vf_mem.pop_back();
        vf_filter_thresh_range.pop_back();
        vf_nentries_range.pop_back();
        vf_ncells_range.pop_back();
        vf_lambda_range.pop_back();
        vf_theta_range.pop_back();
        vf_mor_size_range.pop_back();
        vf_filter_percent_range.pop_back();

        double are_pie_final = are_pies[i] / total_estimated_items;
        double are_mor_final = are_mors[i] / total_estimated_items;
        double insert_tpr_pie = total_packet / total_insert_times_pie[i];
        double insert_tpr_mor = total_packet / total_insert_times_mor[i];
        double query_tpr_pie = total_estimated_items / query_times_pies[i];
        double query_tpr_mor = total_estimated_items / query_times_mors[i];

        cout << RED_TEXT;
        printf("Mem: %6d\tFilter_Thres: %6d\tN_Entries: %6d\tN_Cells: %6d\tLambda: %6d\tTheta: %6d\tMor_Size: %6d\tFilter_Percent: %f\n",
               _mem, _filter_thres, _n_entries, _n_cells, _lambda, _theta, _mor_size, _filter_percent);
        cout << BLUE_TEXT;
        printf("Are_Pie_Final: %12.6f\tInsert_TPR_Pie: %12.6f\tQuery_TPR_Pie: %f\n",
               are_pie_final, insert_tpr_pie, query_tpr_pie);

        printf("Are_Mor_Final: %12.6f\tInsert_TPR_Mor: %12.6f\tQuery_TPR_Mor: %f\n",
               are_mor_final, insert_tpr_mor, query_tpr_mor);

        printf("=================================================\n");
        // resfile<<"FLOW_THRESH,PORTION_THRESH,MEMORY,Filter_THRESH,FILTER_PERCENT,N_CELLS,N_ENTRIES,LAMDBA,THETH,MOR_SIZE"<<endl;

        resfile << flow_thres << "," << portion_thres << "," << _mem << "," << _filter_thres << "," << _filter_percent << "," << _n_cells << "," << _n_entries << "," << _lambda << "," << _theta << "," << _mor_size << "," << are_pie_final << "," << are_mor_final << "," << insert_tpr_pie << "," << query_tpr_pie << ","
                << insert_tpr_mor << "," << query_tpr_mor << endl;
    }
    resfile.close();
}

void Comp_Lat(int MEM_RANGE[], int S1, int FLOW_THRES_CMP, double PORTION_THRES_CMP,
              double FRT, int NET, int NCE, int LAM, int THE, int MORS, int FTHRES)
{
    //! 对比算法的配置

    double f_ratio = FRT;
    int n_et = NET;
    int n_ce = NCE;
    int theta = LAM;
    int lambda = THE;
    int mor_size = MORS;
    int f_thres = FTHRES;
    ////! 对比算法的配置

    PIE_BAISIC<uint64_t> *basics[S1];
    PIE_M1<uint64_t> *mors[S1];
    Hist_01<uint64_t> *hists[S1];
    CMCC<uint64_t> *cmccs[S1];

    for (int i = 0; i < S1; i++)
    {
        basics[i] = new PIE_BAISIC<uint64_t>(MEM_RANGE[i], f_ratio, n_ce, n_et, lambda, theta, f_thres, false);
        hists[i] = new Hist_01<uint64_t>(MEM_RANGE[i], 8, 0.3); // n_slots & cm_ratio,默认配置
        cmccs[i] = new CMCC<uint64_t>(MEM_RANGE[i], 8, 3);
        mors[i] = new PIE_M1<uint64_t>(MEM_RANGE[i], f_ratio, n_ce, n_et, lambda, theta, f_thres, mor_size, false);
    }

    readTraceLat(path, traces);
    int total_packet=traces.size();
    int ss = total_packet;
    cout << "TOTAL ITEMS = " << total_packet << endl;
    map<int, double> total_insert_times_basic;
    map<int, double> total_insert_times_mor;
    map<int, double> total_insert_times_hist;
    map<int, double> total_insert_times_cmcc;

    for (auto it = traces.begin(); it != traces.end(); it++)
    {
        ss--;
        if (ss == 0)
        {
            break;
        }
        uint64_t src = it->src;
        int gap = it->gap + 1;

        // uint16_t srcFp = getFP(src);    // fingerprint
        uint64_t srcFp = src;

        for (int i = 0; i < S1; i++)
        {
            auto t_a = chrono::high_resolution_clock::now();
            basics[i]->insert(srcFp, gap);
            auto t_b = std::chrono::high_resolution_clock::now();
            total_insert_times_basic[i] += chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count();

            auto t_a_1 = chrono::high_resolution_clock::now();
            mors[i]->insert(srcFp, gap);
            auto t_b_1 = std::chrono::high_resolution_clock::now();
            total_insert_times_mor[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_1 - t_a_1).count();

            auto t_a_2 = chrono::high_resolution_clock::now();
            hists[i]->insert(srcFp, gap);
            auto t_b_2 = std::chrono::high_resolution_clock::now();
            total_insert_times_hist[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_2 - t_a_2).count();

            auto t_a_3 = chrono::high_resolution_clock::now();
            cmccs[i]->insert(srcFp, gap);
            auto t_b_3 = std::chrono::high_resolution_clock::now();
            total_insert_times_cmcc[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_3 - t_a_3).count();
        }
        ground_truth_all[src][gap]++;
        ground_truth_size[src]++;
    }

    // basics[0]->show_info();
    // mors[0]->show_info();

    for (auto outerPair : ground_truth_all)
    {
        vector<pair<int, int>> tempVector(outerPair.second.begin(), outerPair.second.end());
        sort(tempVector.begin(), tempVector.end(), comparePairs);
        sorted_ground_truth[outerPair.first] = tempVector;
    }

    // 至此，全部插入完成。 开始查询与写入文件
    map<int, double> are_pies;
    map<int, double> are_mors;
    map<int, double> query_times_pies;
    map<int, double> query_times_mors;
    map<int, double> are_hists;
    map<int, double> are_cmccs;
    map<int, double> query_times_hists;
    map<int, double> query_times_cmccs;
    int total_estimated_flows = 0;
    int total_estimated_items = 0;

    int pie_res;
    int hist_res;
    int cm_res;
    int mor_res;
    for (auto it = sorted_ground_truth.begin(); it != sorted_ground_truth.end(); ++it)
    {
        if (ground_truth_size[it->first] < FLOW_THRES_CMP)
        {
            continue; // 跳过小于FLOW_THRESH的流
        }

        total_estimated_flows++;
        int topV = TOPNUMBER;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {

            if (topV == 0)
            {
                break;
            }
            topV--;
            if (ground_truth_size[it->first] != 0 && (double)it2->second / ground_truth_size[it->first] < PORTION_THRES_CMP)
            {
                break; // 占比小于5%的直接滚蛋！！！
            }
            total_estimated_items++;

            for (int i = 0; i < S1; i++)
            {

                auto t_a_1 = chrono::high_resolution_clock::now();
                pie_res = basics[i]->point_query((it->first), it2->first);
                auto t_b_1 = std::chrono::high_resolution_clock::now();
                query_times_pies[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_1 - t_a_1).count();
                are_pies[i] += abs((double)pie_res - (double)it2->second) / it2->second;

                auto t_a_2 = chrono::high_resolution_clock::now();
                hist_res = hists[i]->point_query((it->first), it2->first);
                auto t_b_2 = std::chrono::high_resolution_clock::now();
                query_times_hists[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_2 - t_a_2).count();
                are_hists[i] += abs((double)hist_res - (double)it2->second) / it2->second;

                auto t_a_3 = chrono::high_resolution_clock::now();
                cm_res = cmccs[i]->point_query((it->first), it2->first);
                auto t_b_3 = std::chrono::high_resolution_clock::now();
                query_times_cmccs[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_3 - t_a_3).count();
                are_cmccs[i] += abs((double)cm_res - (double)it2->second) / it2->second;

                auto t_a_4 = chrono::high_resolution_clock::now();
                mor_res = mors[i]->point_query((it->first), it2->first);
                auto t_b_4 = std::chrono::high_resolution_clock::now();
                query_times_mors[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_4 - t_a_4).count();
                are_mors[i] += abs((double)mor_res - (double)it2->second) / it2->second;
            }
        }
    }
    cout << "total_estimated_flows: " << total_estimated_flows << endl;
    cout << "total_estimated_items: " << total_estimated_items << endl;
    ofstream resfile_cmp(res_path_cmp, ios::app);
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&currentTime);

    // 输出本地时间的各个组成部分
    // std::cout << "Formatted Local Time: ";
    // std::cout << std::put_time(localTime, "%y-%m-%d %H:%M:%S") << std::endl;

    // resfile_cmp << std::put_time(localTime, "%y-%m-%d %H:%M:%S") << endl;
    // resfile_cmp << "FLOW_THRESH = " << FLOW_THRES_CMP << ", PORTION_THRESH=" << PORTION_THRES_CMP << endl;
    // resfile_cmp << "FILTER_RATIO= " << f_ratio << ", FILTER_THRES = " << f_thres << ", N_CELLS = " << n_ce
    //             << ", N_ENTRIES = " << n_et << ", THETA = " << theta << ", LAMBDA = " << lambda << ", "
    //             << ", mor_size = "
    //             << mor_size << endl;

    // resfile_cmp << "MEMORY,ARE_BASIC,ARE_MOR,ARE_HIST,ARE_CMCC,ITPR_BASIC,ITPR_MOR,ITPR_HIST,ITPR_CMCC,QTPR_BASIC,QTPR_MOR,QTPR_HIST,QTPR_CMCC" << endl;

    for (int i = 0; i < S1; i++)
    {
        resfile_cmp << MEM_RANGE[i] << "," << are_pies[i] / total_estimated_items << "," << are_mors[i] / total_estimated_items << "," << are_hists[i] / total_estimated_items << "," << are_cmccs[i] / total_estimated_items
                    << "," << total_packet / total_insert_times_basic[i] << "," << total_packet / total_insert_times_mor[i] << ","
                    << total_packet / total_insert_times_hist[i] << "," << total_packet / total_insert_times_cmcc[i] << "," << total_estimated_items / query_times_pies[i] << "," << total_estimated_items / query_times_mors[i]
                    << "," << total_estimated_items / query_times_hists[i] << "," << total_estimated_items / query_times_cmccs[i] << endl;

        cout << RED_TEXT << endl;
        printf("MEMORY = %d;\n", MEM_RANGE[i]);
        cout << BLUE_TEXT;
        printf("PIE_ARE = %.6f;\t", are_pies[i] / total_estimated_items);
        printf("PIE_INSERT_TPR = %.6fMOPS;\t", total_packet / total_insert_times_basic[i]);
        printf("PIE_QUERY_TPR = %.6fMOPS;\n", total_estimated_items / query_times_pies[i]);

        printf("MOR_ARE = %.6f;\t", are_mors[i] / total_estimated_items);
        printf("MOR_INSERT_TPR = %.6fMOPS;\t", total_packet / total_insert_times_mor[i]);
        printf("MOR_QUERY_TPR = %.6fMOPS;\n", total_estimated_items / query_times_mors[i]);

        printf("Hist_ARE = %.6f;\t", are_hists[i] / total_estimated_items);
        printf("Hist_INSERT_TPR = %.6fMOPS;\t", total_packet / total_insert_times_hist[i]);
        printf("Hist_QUERY_TPR = %.6fMOPS;\n", total_estimated_items / query_times_hists[i]);

        printf("CM_ARE = %.6f;\t", are_cmccs[i] / total_estimated_items);
        printf("CM_INSERT_TPR = %.6fMOPS;\t", total_packet / total_insert_times_cmcc[i]);
        printf("CM_QUERY_TPR = %.6fMOPS;\n", total_estimated_items / query_times_cmccs[i]);
        printf("\n");
    }
}

void Comp_PIE(int MEM_RANGE[], int S1, int FLOW_THRES_PIE, double PORTION_THRES_PIE,
              double FRT, int NET, int NCE, int LAM, int THE, int MORS, int FTHRES)
{
    double f_ratio = FRT;
    int n_et = NET;
    int n_ce = NCE;
    int theta = LAM;
    int lambda = THE;
    int mor_size = MORS;
    int f_thres = FTHRES;

    PIE_BAISIC<uint64_t> *basics[S1];
    PIE_M1<uint64_t> *mors[S1];
    Hist_01<uint64_t> *hists[S1];
    CMCC<uint64_t> *cmccs[S1];

    for (int i = 0; i < S1; i++)
    {
        basics[i] = new PIE_BAISIC<uint64_t>(MEM_RANGE[i], f_ratio, n_ce, n_et, lambda, theta, f_thres, false);
        hists[i] = new Hist_01<uint64_t>(MEM_RANGE[i], 16, 0.5); // n_slots & cm_ratio,默认配置
        cmccs[i] = new CMCC<uint64_t>(MEM_RANGE[i], 16, 3);
        mors[i] = new PIE_M1<uint64_t>(MEM_RANGE[i], f_ratio, n_ce, n_et, lambda, theta, f_thres, mor_size, false);
    }
    traces.clear();
    readTraceLat(path, traces);
    int total_packet = traces.size();
    int ss = total_packet;
    cout << "TOTAL ITEMS = " << total_packet << endl;
    map<int, double> total_insert_times_basic;
    map<int, double> total_insert_times_mor;
    map<int, double> total_insert_times_hist;
    map<int, double> total_insert_times_cmcc;

    for (auto it = traces.begin(); it != traces.end(); it++)
    {
        ss--;
        if (ss == 0)
        {
            break;
        }
        uint64_t src = it->src;
        int gap = it->gap + 1;

        // uint16_t srcFp = getFP(src);    // fingerprint
        uint64_t srcFp = src;

        for (int i = 0; i < S1; i++)
        {
            auto t_a = chrono::high_resolution_clock::now();
            basics[i]->insert(srcFp, gap);
            auto t_b = std::chrono::high_resolution_clock::now();
            total_insert_times_basic[i] += chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count();

            auto t_a_1 = chrono::high_resolution_clock::now();
            mors[i]->insert(srcFp, gap);
            auto t_b_1 = std::chrono::high_resolution_clock::now();
            total_insert_times_mor[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_1 - t_a_1).count();

            auto t_a_2 = chrono::high_resolution_clock::now();
            hists[i]->insert(srcFp, gap);
            auto t_b_2 = std::chrono::high_resolution_clock::now();
            total_insert_times_hist[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_2 - t_a_2).count();

            auto t_a_3 = chrono::high_resolution_clock::now();
            cmccs[i]->insert(srcFp, gap);
            auto t_b_3 = std::chrono::high_resolution_clock::now();
            total_insert_times_cmcc[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_3 - t_a_3).count();
        }
        ground_truth_all[src][gap]++;
        ground_truth_size[src]++;
    }

    for (auto outerPair : ground_truth_all)
    {
        vector<pair<int, int>> tempVector(outerPair.second.begin(), outerPair.second.end());
        sort(tempVector.begin(), tempVector.end(), comparePairs);
        sorted_ground_truth[outerPair.first] = tempVector;
    }

    map<int, double> pre_basic;
    map<int, double> pre_basic_portion;
    map<int, double> pre_hist;
    map<int, double> pre_hist_portion;
    map<int, double> pre_cmcc;
    map<int, double> pre_cmcc_portion;
    map<int, double> pre_mor;
    map<int, double> pre_mor_portion;

    map<int, double> p_total_query_time_basic;
    map<int, double> p_total_query_time_hist;
    map<int, double> p_total_query_time_cmcc;
    map<int, double> p_total_query_time_mor;

    vector<pair<int, int>> pie_of_baisc;
    vector<pair<int, int>> pie_of_hist;
    vector<pair<int, int>> pie_of_cmcc;
    vector<pair<int, int>> pie_of_mor;
    vector<pair<int, int>> truth_vector;
    int total_estimated_flows_pie = 0;

    for (auto it = sorted_ground_truth.begin(); it != sorted_ground_truth.end(); ++it)
    {
        if (ground_truth_size[it->first] < FLOW_THRES_PIE)
        {
            continue;
        }
        total_estimated_flows_pie++; 
        truth_vector = it->second;

        for (int i = 0; i < S1; i++)
        {
            auto t_a_1 = chrono::high_resolution_clock::now();
            pie_of_baisc = basics[i]->pie_query(it->first);
            auto t_b_1 = std::chrono::high_resolution_clock::now();
            p_total_query_time_basic[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_1 - t_a_1).count();

            auto t_a_2 = chrono::high_resolution_clock::now();
            pie_of_hist = hists[i]->pie_query(it->first);
            auto t_b_2 = std::chrono::high_resolution_clock::now();
            p_total_query_time_hist[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_2 - t_a_2).count();

            auto t_a_3 = chrono::high_resolution_clock::now();
            pie_of_cmcc = cmccs[i]->pie_query(it->first);
            auto t_b_3 = std::chrono::high_resolution_clock::now();
            p_total_query_time_cmcc[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_3 - t_a_3).count();

            auto t_a_4 = chrono::high_resolution_clock::now();
            pie_of_mor = mors[i]->pie_query(it->first);
            auto t_b_4 = std::chrono::high_resolution_clock::now();
            p_total_query_time_mor[i] += chrono::duration_cast<std::chrono::microseconds>(t_b_4 - t_a_4).count();

            pre_basic[i] += calculateTotalDifference(it->first, truth_vector, pie_of_baisc, PORTION_THRES_PIE);
            pre_hist[i] += calculateTotalDifference(it->first, truth_vector, pie_of_hist, PORTION_THRES_PIE);
            pre_cmcc[i] += calculateTotalDifference(it->first, truth_vector, pie_of_cmcc, PORTION_THRES_PIE);
            pre_mor[i] += calculateTotalDifference(it->first, truth_vector, pie_of_mor, PORTION_THRES_PIE);

            //! 百分比误差，计算CC所占百分比的误差
            pre_basic_portion[i] += calculatePortionDifference(it->first, truth_vector, pie_of_baisc, PORTION_THRES_PIE);
            pre_hist_portion[i] += calculatePortionDifference(it->first, truth_vector, pie_of_hist, PORTION_THRES_PIE);
            pre_cmcc_portion[i] += calculatePortionDifference(it->first, truth_vector, pie_of_cmcc, PORTION_THRES_PIE);
            pre_mor_portion[i] += calculatePortionDifference(it->first, truth_vector, pie_of_mor, PORTION_THRES_PIE);
        }
    }

    ofstream resfile_cmp_pie(res_path_cmp_pie, ios::app);
    chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t currentTime = std::chrono::system_clock::to_time_t(now);
    tm *localTime = std::localtime(&currentTime);

    // 输出本地时间的各个组成部分
    std::cout << "Formatted Local Time: ";
    std::cout << std::put_time(localTime, "%y-%m-%d %H:%M:%S") << std::endl;

    resfile_cmp_pie << std::put_time(localTime, "%y-%m-%d %H:%M:%S") << endl;
    resfile_cmp_pie << "FLOW_THRESH = " << FLOW_THRES_PIE << ", PORTION_THRESH=" << PORTION_THRES_PIE << endl;
    resfile_cmp_pie << "FILTER_RATIO= " << f_ratio << ", FILTER_THRES = " << f_thres << ", N_CELLS = " << n_ce
                    << ", N_ENTRIES = " << n_et << ", THETA = " << theta << ", LAMBDA = " << lambda << ", "
                    << ", mor_size = "
                    << mor_size << endl;
    resfile_cmp_pie << "MEMORY,PRE_BASIC,PRE_MOR,PRE_HIST,PRE_CMCC,PORTIONRE_BASIC,PORTIONRE_MOR,PORTIONRE_HIST,PORTIONRE_CMCC,P_QTPR_BASIC,P_QTPR_MOR,P_QTPR_HIST,P_QTPR_CMCC" << endl;

    for (int i = 0; i < S1; i++)
    {
        resfile_cmp_pie << MEM_RANGE[i] << "," << pre_basic[i] / total_estimated_flows_pie << ","
                        << pre_mor[i] / total_estimated_flows_pie << ","
                        << pre_hist[i] / total_estimated_flows_pie << ","
                        << pre_cmcc[i] / total_estimated_flows_pie << ","
                        << pre_basic_portion[i] / total_estimated_flows_pie << ","
                        << pre_mor_portion[i] / total_estimated_flows_pie << ","
                        << pre_hist_portion[i] / total_estimated_flows_pie << ","
                        << pre_cmcc_portion[i] / total_estimated_flows_pie << ","
                        << total_estimated_flows_pie / p_total_query_time_basic[i] << ","
                        << total_estimated_flows_pie / p_total_query_time_mor[i] << ","
                        << total_estimated_flows_pie / p_total_query_time_hist[i] << ","
                        << total_estimated_flows_pie / p_total_query_time_cmcc[i] << endl;

        cout << RED_TEXT << endl;
        printf("MEMORY = %d;\n", MEM_RANGE[i]);
        cout << BLUE_TEXT;
        printf("PIE_PRE = %.6f;\t", pre_basic[i] / total_estimated_flows_pie);
        printf("PIE_PORTION_RE = %.6f;\t", pre_basic_portion[i] / total_estimated_flows_pie);
        printf("PIE_QUERY_TPR = %.6fMOPS;\n", total_estimated_flows_pie / p_total_query_time_basic[i]);

        printf("MOR_PRE = %.6f;\t", pre_mor[i] / total_estimated_flows_pie);
        printf("MOR_PORTION_RE = %.6f;\t", pre_mor_portion[i] / total_estimated_flows_pie);
        printf("MOR_QUERY_TPR = %.6fMOPS;\n", total_estimated_flows_pie / p_total_query_time_mor[i]);

        printf("Hist_PRE = %.6f;\t", pre_hist[i] / total_estimated_flows_pie);
        printf("Hist_PORTION_RE = %.6f;\t", pre_hist_portion[i] / total_estimated_flows_pie);
        printf("Hist_QUERY_TPR = %.6fMOPS;\n", total_estimated_flows_pie / p_total_query_time_hist[i]);

        printf("CM_PRE = %.6f;\t", pre_cmcc[i] / total_estimated_flows_pie);
        printf("CM_PORTION_RE = %.6f;\t", pre_cmcc_portion[i] / total_estimated_flows_pie);
        printf("CM_QUERY_TPR = %.6fMOPS;\n", total_estimated_flows_pie / p_total_query_time_cmcc[i]);
        printf("\n");
    }
}

int main()
{

    // // // //!  参数调节！
    // int param_mem_range[] = {300};
    // int param_filter_thresh_range[] = {1};
    // double param_filter_percent_range[] = {0.01};      //! 删除了过滤器！
    // int param_theta_range[] = {1,2,3,4,5};        // 3
    // int param_lambda_range[] = {2};       // 2
    // int param_ncells_range[] = {32};      // 32
    // int param_nentries_range[] = {8};     //   低内存4，中内存8，高内存12
    // int param_mor_size_range[] = {8};     // 8
    // int param_flow_thres = 50;
    // double param_portion_thres = 0.1;
    // //!  参数调节！

    // size_t size_mem = std::size(param_mem_range);
    // size_t size_filter_thresh = std::size(param_filter_thresh_range);
    // size_t size_theta = std::size(param_theta_range);
    // size_t size_lambda = std::size(param_lambda_range);
    // size_t size_ncells = std::size(param_ncells_range);
    // size_t size_nentries = std::size(param_nentries_range);
    // size_t size_mor_size = std::size(param_mor_size_range);
    // size_t size_filter_percent = std::size(param_filter_percent_range);
    // Find_Best_Param(param_mem_range, size_mem, param_filter_thresh_range, size_filter_thresh,
    //                 param_theta_range, size_theta, param_lambda_range, size_lambda, param_nentries_range, size_nentries,
    //                 param_ncells_range, size_ncells,
    //                 param_mor_size_range, size_mor_size, param_filter_percent_range, size_filter_percent,
    //                 param_flow_thres, param_portion_thres);

    double f_ratio = 0.01;
    int f_thres = 0; //! 实际已经删除了过滤器
    int n_ce = 16;
    int n_et = 4; 
    int theta = 3;
    int lambda = 2;  
    int mor_size = 8; 
    int cmp_mem_range[] = {800};
    int cmp_flow_thres = 50; // 事实上,对于seattle, 是十分确定的！ 每个key的freq都是600
    double cmp_portion_thres = 0.1;
    // Comp_Lat(cmp_mem_range, size(cmp_mem_range), cmp_flow_thres, cmp_portion_thres,
    // f_ratio, n_et, n_ce, lambda, theta, mor_size, f_thres);

    int pie_cmp_mem_range[] = {50, 100, 150, 200, 250, 300, 400, 500, 600, 700, 800};
    Comp_PIE(pie_cmp_mem_range, size(pie_cmp_mem_range), cmp_flow_thres, cmp_portion_thres, f_ratio, n_et, n_ce, lambda, theta, mor_size, f_thres);

    return 0;
}

