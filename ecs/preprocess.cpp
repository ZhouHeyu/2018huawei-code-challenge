//
// Created by zj on 18-5-14.
//

#include "preprocess.h"
#include <malloc.h>
#include <string.h>
#include <algorithm>
#include "GGA.h"
#include <assert.h>
#include <fstream>

/*********************************STL的操作****************************/
typedef pair<int, int> PAIR;

struct CmpByValueDescend {
    bool operator()(const PAIR &lhs, const PAIR &rhs) {
        return lhs.second > rhs.second;
    }
};

struct CmpByValueAscend {
    bool operator()(const PAIR &lhs, const PAIR &rhs) {
        return lhs.second < rhs.second;
    }
};
/*****************************************************************/


/********************删除最大的异常值高分******************************/
int DelMaxOutliers(int *ori_data, int ori_data_length, double max_th, int min_value) {
    map<int, int> index_and_value;
    index_and_value.clear();
    for (int i = 0; i < ori_data_length; ++i) {
        index_and_value[i] = ori_data[i];
    }
    vector<PAIR> index_and_value_vec(index_and_value.begin(), index_and_value.end());
    sort(index_and_value_vec.begin(), index_and_value_vec.end(), CmpByValueDescend());
    int *to_del = (int *) malloc(sizeof(int) * ori_data_length);
    if (to_del == nullptr) {
        fprintf(stderr, "malloc for to_del arr is failed\n");
        return 1;
    }
    memset(to_del, -1, sizeof(int) * ori_data_length);
    int now = 0;
    int max_value = index_and_value_vec[0].second;
    for (int j = 0; j < index_and_value_vec.size(); ++j) {
        int index = index_and_value_vec[j].first;
        int value = index_and_value_vec[j].second;
        if (j + 1 < index_and_value_vec.size() && index_and_value_vec[j + 1].second == 0) {
            max_value = index_and_value_vec[j].second;
            break;
        }
        if (j + 1 < index_and_value_vec.size() &&
            index_and_value_vec[j].second * 1.0 / index_and_value_vec[j + 1].second < max_th) {
            max_value = index_and_value_vec[j].second;
            break;
        }
        if (index_and_value_vec[j].second < min_value) {
            max_value = index_and_value_vec[j].second;
            break;
        }
        to_del[now] = index_and_value_vec[j].first;
        now++;

    }

//    异常值补填
    for (int k = 0; k < now; ++k) {
        int del_index = to_del[k];
        ori_data[del_index] = max_value;
    }

    return 0;

}

////////全局变量
limit_info g_limit_infos[LIMIT_INFO_COUNT];//物理机信息
vector<int> g_flavor_histories[MAX_FLAVOR_COUNT];// 历史数据：　24*训练天数　的矩阵
double g_flavor_prices[MAX_FLAVOR_COUNT];// 虚拟机价格，-1代表无需预测
time_t g_ori_time;//2010-01-01
int g_pred_begin_day; //预测开始天
int g_pred_end_day;//预测结束天

/////////初始化部分全局变量
void init_global_vars() {
    for (int i = 0; i < LIMIT_INFO_COUNT; i++) {
        g_limit_infos[i].cpu_value = g_limit_infos[i].mem_value = -1;
    }
    struct tm ori_tm = {0, 0, 0, 1, 1, 2010};
    g_ori_time = mktime(&ori_tm);
    for (int i = 0; i < MAX_FLAVOR_COUNT; i++) {
        g_flavor_prices[i] = -1;
    }
}

int glh2index(char glh) {
    switch (glh) {
        case 'G':
            return 0;
        case 'L':
            return 1;
        case 'H':
            return 2;
        default:
            return -1;
    }
}

/////////日期字符串转天数
int get_days(const char *date) {
    tm tm = {0};
    sscanf(date, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    return (int) difftime(mktime(&tm), g_ori_time) / (60 * 60 * 24);//TODO
}

/*
 * 解析input文件，填充以下全局变量：
 *  g_limit_infos　
 *  g_flavor_prices
 *  g_pred_begin_day
 *  g_pred_end_day
*/
void parse_input(char **info, int info_line_num) {
    int input_limit_count = atoi(info[0]);
    for (int i = 1; i < 1 + input_limit_count; i++) {
        char *line = info[i];
        int index = glh2index(line[0]);
        assert(index >= 0);
        g_limit_infos[index].pyhsical_Name = line[0];
        sscanf(line, "%*[a-zA-Z] %d %d %*d %lf",
               &g_limit_infos[index].cpu_value,
               &g_limit_infos[index].mem_value,
               &g_limit_infos[index].money);
    }
    int flovors_count = atoi(info[input_limit_count + 2]);
    for (int i = input_limit_count + 3; i < input_limit_count + 3 + flovors_count; i++) {
        char *line = info[i];
        int flovor;
        double price;
        sscanf(line, "flavor%d %*d %*d %lf",
               &flovor,
               &price);
        assert(flovor > 0);
        g_flavor_prices[flovor - 1] = price;
        printf("flovor%d : %lf\n", flovor, price);
    }
    g_pred_begin_day = get_days(info[input_limit_count + 3 + flovors_count + 1]);
    g_pred_end_day = get_days(info[input_limit_count + 3 + flovors_count + 2]);
}

/*
 * 解析训练文件，填充以下全局变量：
 *  g_flavor_histories　: 24*训练天数　矩阵
*/
void parse_data(char **data, int data_num) {
    int days_begin, days_end;
    char date_buf[16];
    char flavor_buf[8];
    sscanf(data[0], "%*s %*s %s", date_buf);
    days_begin = get_days(date_buf);
    sscanf(data[data_num - 1], "%*s %*s %s", date_buf);
    days_end = get_days(date_buf);
    int days_gap = days_end - days_begin + 1;
    assert(days_gap > 0);
    for (int i = 0; i < MAX_FLAVOR_COUNT; i++) {
        g_flavor_histories[i] = vector<int>((size_t) (days_gap), 0);
    }
    for (int i = 0; i < data_num; ++i) {
        char *line = data[i];
        int flavor;
        sscanf(line, "%*s flavor%s %s",
               flavor_buf,
               date_buf);
        flavor = atoi(flavor_buf);
        assert(flavor > 0);
        int days = get_days(date_buf);
        assert(days > 0);
        g_flavor_histories[flavor - 1][days - days_begin]++;
    }
    int last_valid_column = 0;
    for (int j = 0; j < days_gap; j++) {
        for (int i = 0; i < MAX_FLAVOR_COUNT; i++) {
            if (g_flavor_histories[i][j] > 0) {
                last_valid_column = j;
                break;
            }
        }
        for (int i = 0; i < MAX_FLAVOR_COUNT; i++) {
            g_flavor_histories[i][j] = g_flavor_histories[i][last_valid_column];
        }
    }
}

//DEBUG
void dump_history_to_file() {
    ofstream output_stream;
    output_stream.open("/tmp/history.dump.c.txt");
    for (int i = 0; i < MAX_FLAVOR_COUNT; i++) {
        for (int j = 0; j < g_flavor_histories[i].size(); j++) {
            output_stream << g_flavor_histories[i][j];
            if (j != g_flavor_histories[i].size() - 1) {
                output_stream << ",";
            }
        }
        output_stream << '\n';
    }
    output_stream.close();
}