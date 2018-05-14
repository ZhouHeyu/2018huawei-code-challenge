#include "predict.h"
#include "preprocess.h"
#include "GGA.h"
#include "ES.h"
int pre_flavor_num[24];

//TODO:
void result_to_string(Result result) {

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
            int pre_num=0;
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

//    dump_history_to_file();
    char *result_file = (char *) "17\n\n0 8 0 20";
    write_result(result_file, filename);
}
