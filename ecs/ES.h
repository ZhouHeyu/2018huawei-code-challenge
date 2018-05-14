//
// Created by zj on 18-5-14.
/***************用以实现三次指数平滑的函数************************/

#ifndef ECS_ES_H
#define ECS_ES_H

#include <iostream>
#include <math.h>

// 三次指数平滑的系数计算
int ES_three(double *A,double *B,double *C,const int *ori_data,int ori_data_length,int init_num,double a);
double *ES_predict(const int *ori_data,int ori_data_length,int T_diff,double *pre_data,int predict_len,double a=0.3);



#endif //ECS_ES_H
