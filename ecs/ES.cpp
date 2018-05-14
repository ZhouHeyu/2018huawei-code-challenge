//
// Created by zj on 18-5-14.
//

#include "ES.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

int ES_three(double *A,double *B,double *C,const int *ori_data,int ori_data_length,int init_num=3,double a=0.5)
{

//    一次指数平滑计算S1
    double *S1=(double *)malloc(sizeof(double)*ori_data_length);
    if(S1== nullptr){
        fprintf(stderr,"malloc for S1 failed\n");
        exit(1);
    }
    memset(S1,0,sizeof(double)*ori_data_length);
    double S0=0.0;
    for (int i = 0; i <init_num ; ++i) {
        S0+=ori_data[i];
    }
    S0=S0/init_num;
    double temp1=S0;
    for (int j = 0; j <ori_data_length ; ++j) {
        double temp2=a*ori_data[j]+(1-a)*temp1;
        S1[j]=temp2;
        temp1=temp2;
    }
//　二次指数平滑计算
    double *S2=(double *)malloc(sizeof(double)*ori_data_length);
    if(S2== nullptr){
        fprintf(stderr,"malloc for S2 failed\n");
        exit(1);
    }
    memset(S2,0, sizeof(double)*ori_data_length);
    double S00=0.0;
    int init_num2=1;
    for (int k = 0; k < init_num2; ++k) {
        S00+=S1[k];
    }
    S00=S00/init_num2;
    temp1=S00;
    for (int l = 0; l <ori_data_length ; ++l) {
        double temp2=a*S1[l]+(1-a)*temp1;
        S2[l]=temp2;
        temp1=temp2;
    }
    //　三次指数平滑计算
    double *S3=(double *)malloc(sizeof(double)*ori_data_length);
    if(S3== nullptr){
        fprintf(stderr,"malloc for S3 failed\n");
        exit(1);
    }
    double S000=0.0;
    int init_num3=1;
    for (int m = 0; m <init_num3 ; ++m) {
        S000+=S2[m];
    }
    temp1=S000/init_num3;
    for (int n = 0; n <ori_data_length ; ++n) {
        double temp2=a*S2[n]+(1-a)*temp1;
        S3[n]=temp2;
        temp1=temp2;
    }

//    根据公式计算对应的Ａ,B.C
    int tail=ori_data_length-1;
    *A=3*S1[tail]-3*S2[tail]+S3[tail];
    *B=((6-5*a)*S1[tail]-2*(5-4*a)*S2[tail]+(4-3*a)*S3[tail])*a/(2*pow((1-a),2));
    *C=(S1[tail]-2*S2[tail]+S3[tail])*a*a/(2*pow((1-a),2));

//    注意释放内存
    free(S1);
    free(S2);
    free(S3);

    return 0;

}

//函数返回一个double指针
 double *ES_predict(const int *ori_data,int ori_data_length,int T_diff,double *pre_data,int predict_len,double a=0.3)
 {
     double A=0.0,B=0.0,C=0.0;
     ES_three(&A,&B,&C,ori_data,ori_data_length,3,a);

     for (int i = 0; i <predict_len ; ++i) {
            int T=T_diff+i;
            double pre=A+B*(T+1)+C*pow((T+1),2);
            pre_data[i]=pre;
     }
     return pre_data;
 }