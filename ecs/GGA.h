//
// Created by zj on 18-5-10.
//

#ifndef GGA_GGA_H
#define GGA_GGA_H
#include <stdio.h>
#include <malloc.h>
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include <map>
#include <string.h>
using namespace std;

#define PHYSICAL_MAX_CONTAIN 200
#define MAX_PHYSICAL_NUM 8000

const int limit_type_num=24;

/*******************关键数据结构声明***********************************/

//初始化时，cpu_value,mem_max为－１即为无效数据
typedef struct limit_info{
    char pyhsical_Name;
    int cpu_value;
    int mem_value;
    double money;
}limit_info;

//设置单个物理的放置情况的结构体
struct Physical_Node{
    int use_flag; //unused is 0;used is 1
    int curr_contain_num;
    int Physical_ID;
    int cpu_max;
    int mem_max;
    int remain_cpu;
    int remain_mem;
    double curr_price;
    int contain_Template_Index[PHYSICAL_MAX_CONTAIN];
};

//设置单个解所包含的物理机的情况
typedef struct  pop_individual{
    int curr_physical_num;
    double curr_cost;
    int group_count_list[3];
    struct Physical_Node Physical_play_list[MAX_PHYSICAL_NUM];
}pop_individual;


//定义对应的预测虚拟机的数量和类型
typedef struct pre_flavor_info{
    int flavor_num[limit_type_num];
    double flavor_cost[limit_type_num];
}pre_flavor_info;

//遗传算法返回的结构体, 如果contain_flavor_type_num[i]＜＝0,则不需要输出其值
typedef struct result_physical_node{
    int contain_flavor_type_num[limit_type_num];
}result_physical_node;


//该结构体返回时，针对H_Need_Num =0 为异常值，则不要需要遍历H_Need_list
typedef struct Result{
    int H_Need_Num;
    int L_Need_Num;
    int G_Need_Num;
    result_physical_node H_Need_list[MAX_PHYSICAL_NUM];
    result_physical_node L_Need_list[MAX_PHYSICAL_NUM];
    result_physical_node G_Need_list[MAX_PHYSICAL_NUM];
}Result;


/*********************全局变量申明**************************************/
//组合排列的模板队列
extern int *Template;
extern int Template_size;

//遗传算法使用的种群大小，选择概率列表，适应度数组
extern double *possbile_list;
extern int pop_size;
extern double *fit_value_list;
extern double all_profit;
//设置一个解的动态数组全局变量
extern pop_individual * pop_list;


/**************************函数声明*********************************/

/****************************通用函数***************************/
//初始化函数声明
void init(int size);
void end();
//约束条件生成
void SelectMustList();
//轮盘赌
double myrand();
double *cumsum(const double *p_list,int size);
int Select(int size);
//找到空闲的位置(-1)
int find_free_pos(const int *a,int size);
int find_value_in_arr(const int* a,int a_size,int value);
/******************************测试函数*****************************/
void testprint(int size);
/*************************结构体初始化函数**************************/
//初始化结构体的函数
void Physical_Node_Init(Physical_Node *p);
void pop_individual_init(pop_individual *p,bool);
//在外部读取前,初始化对应的全局变量out_flavor_info
void InitOutFlavorInfo();
//在外部读取前,初始化对应的全局变量数组Physical_Info_Arr[3];
void InitPhysicalInfoArr();
//根据外部参数out_flavor_info，设计对应的Template和对应的flavor_cost
void SetTemplateAndFlavorCost();
/*************************遗传算法的关键步骤****************************/
//初始化种群的采用的是ＦＦ算法
void myffd(pop_individual&,const int *a,int a_size);
void POP_Init(int POP_size,const int *Template,int Template_size);
/*******************交叉步骤***************************/
void MRC(pop_individual&,int X_index,int Y_index,int Cross_Num);
/*******************变异步骤**************************/
void MRM(pop_individual&,int X_index,int Varition_num);
/*******************适应度函数**************************/
double git_fit_value(const pop_individual &p);
//计算全局包含成本的销售额
void ComputeAllProfit();
//遗传算法的主函数
Result GAA_main(int * pre_flavor_arr,int pre_arr_size,const pre_flavor_info flavor_info,const limit_info *physical_info,const int physical_info_size,const int size,const int max_iter,const double Cross_rate,const double Varition_rate,const double C_ratio,const double D_ratio);

#endif //GGA_GGA_H
