//
// Created by zj on 18-5-14.
//

#include "preprocess.h"
#include <malloc.h>
#include <string.h>
#include <algorithm>

/*********************************STL的操作****************************/
typedef pair<int,int> PAIR;

struct CmpByValueDescend{
    bool operator()(const PAIR &lhs,const PAIR &rhs){
        return lhs.second>rhs.second;
    }
};

struct CmpByValueAscend{
    bool operator()(const PAIR &lhs,const PAIR &rhs){
        return lhs.second<rhs.second;
    }
};
/*****************************************************************/


/********************删除最大的异常值高分******************************/
int DelMaxOutliers(int *ori_data,int ori_data_length,double max_th,int min_value)
{
    map<int,int> index_and_value;
    index_and_value.clear();
    for (int i = 0; i <ori_data_length ; ++i) {
        index_and_value[i]=ori_data[i];
    }
    vector<PAIR> index_and_value_vec(index_and_value.begin(),index_and_value.end());
    sort(index_and_value_vec.begin(),index_and_value_vec.end(),CmpByValueDescend());
    int *to_del=(int *)malloc(sizeof(int)*ori_data_length);
    if(to_del== nullptr){
        fprintf(stderr,"malloc for to_del arr is failed\n");
        return 1;
    }
    memset(to_del,-1,sizeof(int)*ori_data_length);
    int now=0;
    int max_value=index_and_value_vec[0].second;
    for (int j = 0; j <index_and_value_vec.size(); ++j) {
        int index= index_and_value_vec[j].first;
        int value= index_and_value_vec[j].second;
        if(j+1<index_and_value_vec.size() && index_and_value_vec[j+1].second==0){
            max_value=index_and_value_vec[j].second;
            break;
        }
        if(j+1<index_and_value_vec.size() && index_and_value_vec[j].second*1.0/index_and_value_vec[j+1].second<max_th){
            max_value=index_and_value_vec[j].second;
            break;
        }
        if(index_and_value_vec[j].second<min_value){
            max_value=index_and_value_vec[j].second;
            break;
        }
        to_del[now]=index_and_value_vec[j].first;
        now++;

    }

//    异常值补填
    for (int k = 0; k < now; ++k) {
        int del_index=to_del[k];
        ori_data[del_index]=max_value;
    }

    return 0;

}
