//
// Created by zj on 18-5-14.
/***************该模块函数实现异常值的处理，缺失值的补填************/
//

#ifndef ECS_PREPROCESS_H
#define ECS_PREPROCESS_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "GGA.h"

using namespace std;

#define LIMIT_INFO_COUNT 3
#define MAX_FLAVOR_COUNT 24

//删除最大的异常值高分
int DelMaxOutliers(int *ori_data, int ori_data_length, double max_th, int min_value);

void parse_data(char **data, int data_num);

void parse_input(char **info, int info_line_num);

void init_global_vars();

void dump_history_to_file();

////////全局变量
extern limit_info g_limit_infos[];//物理机信息
extern vector<int> g_flavor_histories[];// 历史数据：　24*训练天数　的矩阵
extern double g_flavor_prices[];// 虚拟机价格，-1代表无需预测
extern time_t g_ori_time;//2010-01-01
extern int g_pred_begin_day; //预测开始天
extern int g_pred_end_day;//预测结束天

#endif //ECS_PREPROCESS_H
