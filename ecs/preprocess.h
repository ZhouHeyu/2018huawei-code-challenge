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
using namespace std;



//删除最大的异常值高分
int DelMaxOutliers(int *ori_data,int ori_data_length,double max_th,int min_value);



#endif //ECS_PREPROCESS_H
