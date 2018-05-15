#include "predict.h"
#include "preprocess.h"
#include "GGA.h"
#include "ES.h"
#include <sstream>

int pre_flavor_num[24];

void _write_phy_result(stringstream &output,
                       const char *name,
                       int count,
                       result_physical_node *results) {
    if (count <= 0) {
        return;
    }
    output << name << " " << count << endl;
    for (int i = 0; i < count; i++) {
        result_physical_node one_node = results[i];
        output << name << '-' << i + 1;
        for (int j = 0; j < MAX_FLAVOR_COUNT; j++) {
            if (one_node.contain_flavor_type_num[j] > 0) {
                output << " flavor" << j + 1 << " " << one_node.contain_flavor_type_num[j];
            }
        }
        output << endl;
    }
    output << endl;
}

void write_result(Result result, const char *const fname) {
    stringstream output;
    int total_vm_pred = 0;
    for (int i = 0; i < MAX_FLAVOR_COUNT; ++i) {
        if (pre_flavor_num[i] > 0) {
            total_vm_pred += pre_flavor_num[i];
        }
    }
    output << total_vm_pred << endl;
    for (int i = 0; i < MAX_FLAVOR_COUNT; ++i) {
        if (pre_flavor_num[i] >= 0) {
            output << "flavor" << i + 1 << " " << pre_flavor_num[i] << endl;
        }
    }
    output << endl;
    _write_phy_result(output, "General", result.G_Need_Num, result.G_Need_list);
    _write_phy_result(output, "Large-Memory", result.L_Need_Num, result.L_Need_list);
    _write_phy_result(output, "High-Performance", result.H_Need_Num, result.H_Need_list);
    write_result(trim_right(output.str(),"\n").data(), fname);
}

void predict_server(char *info[MAX_INFO_NUM], int info_line_num,
                    char *data[MAX_DATA_NUM], int data_num, char *filename) {

    init_global_vars();
    parse_input(info, info_line_num);
    parse_data(data, data_num);
//    ?????????
    for (int i = 0; i <24 ; ++i) {
        DelMaxOutliers(g_flavor_histories[i].data(),g_flavor_histories[i].size(),10.0,4);
    }
//    ????????
    memset(pre_flavor_num,-1, sizeof(int)*24);
    int pre_length=g_pred_end_day-g_pred_begin_day+1;
    int day_interval=g_pred_begin_day-g_train_end_day+1;
/**********************?????????******************************************/
    double cc[24];
    if(pre_length<=13){
        cc[0] = 0.85;
        cc[1] = 1.3;
        cc[2] = 1.4;
        cc[3] = 0.35;
        cc[4] = 0.9;
        cc[5] = 0.5;
        cc[6] = 1.0;
        cc[7] = 0.9;
        cc[8] = 1.1;
        cc[9] = 1.0;
        cc[10] = 1.0;
        cc[11] = 0.8;
        cc[12] = 1.0;
        cc[13] = 1.3;
        cc[14] = 1.0;
        cc[15] = 1.5;
        cc[16] = 1.0;
        cc[17] = 1.0;
        cc[18] = 1.0;
        cc[19] = 1.0;
        cc[20] = 1.0;
        cc[21] = 1.0;
        cc[22] = 1.0;
        cc[23] = 1.0;
    }else{
        cc[0] = 1.0;
        cc[1] = 1.3;
        cc[2] = 1.1;
        cc[3] = 0.7;
        cc[4] = 0.8;
        cc[5] = 1.0;
        cc[6] = 1.0;
        cc[7] = 1.0;
        cc[8] = 0.9;
        cc[9] = 1.0;
        cc[10] = 1.1;
        cc[11] = 1.0;
        cc[12] = 1.0;
        cc[13] = 1.5;
        cc[14] = 1.0;
        cc[15] = 1.5;
        cc[16] = 1.0;
        cc[17] = 1.0;
        cc[18] = 0.9;
        cc[19] = 1.0;
        cc[20] = 1.0;
        cc[21] = 1.0;
        cc[22] = 1.0;
        cc[23] = 1.0;
    }

/**********************?????******************************************/
    double a[24];
    a[0]=0.028;
    a[1]= 0.03;
    a[2]=0.03;
    a[3]=0.03;
    a[4]=0.03;
    a[5]=0.03;
    a[6]=0.03;
    a[7]=0.03;
    a[8]=0.058;
    a[9]=0.03;
    a[10]=0.03;
    a[11]= 0.03;
    a[12]=0.03;
    a[13]=0.03;
    a[14]=0.03;
    a[15]=0.03;
    a[16]=0.03;
    a[17]=0.03;
    a[18]=0.03;
    a[19]=0.03;
    a[20]=0.03;
    a[21]=0.03;
    a[22]=0.03;
    a[23]=0.03;
/**********************????b??******************************************/
    double bb[24];
    bb[0]=0;
    bb[1]=0;
    bb[2]=0;
    bb[3]=0;
    bb[4]=0;
    bb[5]=0;
    bb[6]=0;
    bb[7]=0;
    bb[8]=0;
    bb[9]=0;
    bb[10]=0;
    bb[11]=0;
    bb[12]=0;
    bb[13]=0;
    bb[14]=0;
    bb[15]=0;
    bb[16]=0;
    bb[17]=0;
    bb[18]=0;
    bb[19]=0;
    bb[20]=0;
    bb[21]=0;
    bb[22]=0;
    bb[23]=0;

/************************??????*************************************/

    for (int j = 0; j <24 ; ++j) {
        if(g_flavor_prices[j]>=0){
//            int pre_num=0;
            double *pre_data=(double*)malloc(sizeof(double)*pre_length);
            if(pre_data== nullptr){
                fprintf(stderr,"malloc for pre data arr is failed\n");
                exit(1);
            }
            memset(pre_data,0,sizeof(double)*pre_length);
            ES_predict(g_flavor_histories[j].data(),g_flavor_histories[j].size(),day_interval,pre_data,pre_length,a[j]-0.013);
            double temp=0.0;
            for (int i = 0; i <pre_length ; ++i) {
                temp+=pre_data[i];
            }
            temp=temp*cc[j]+bb[j];
            if(temp<0.5){
                temp=0;
            }else if(temp>0.5&&temp<1){
                temp=1;
            }
            pre_flavor_num[j]=(int)temp;
        }
    }

//    ????????????????????

//    dump_history_to_file();
//    Result result = {2, 0, 3};
//    result.H_Need_list[0].contain_flavor_type_num[0] = 1;
//    result.H_Need_list[0].contain_flavor_type_num[1] = 2;
//    result.H_Need_list[1].contain_flavor_type_num[1] = 3;
    Result r;
    int pop_size=5;
    int max_iter=3;
    double cross_rate=0.7;
    double varition_rate=0.1;
    double C_rate=0.5;
    double D_rate=0.2;
    pre_flavor_info t_flavor_info;
    for (int k = 0; k <24 ; ++k) {
        if(g_flavor_prices[k]>0){
            t_flavor_info.flavor_num[k]=pre_flavor_num[k];
            t_flavor_info.flavor_cost[k]=g_flavor_prices[k];
        }else{
            t_flavor_info.flavor_cost[k]=-1.0;
            t_flavor_info.flavor_num[k]=-1;
        }
    }

    r=GAA_main(pre_flavor_num,24,t_flavor_info,g_limit_infos,3,pop_size,max_iter,cross_rate,varition_rate,C_rate,D_rate);
    write_result(r, filename);
}
