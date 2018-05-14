//
// Created by zj on 18-5-10.
//
#include "GGA.h"
#include <iostream>
using namespace std;
/*********************全局变量申明**************************************/
//从外部读取的物理机信息结构体数组
limit_info Physical_Info_Arr[3];

vector<int> Template_index_list;


//对应必须防止的约束条件队列
vector<int> H_must_list;
vector<int> L_must_list;
//设置对应的选择
vector<int> select_list;
//用以查询对应虚拟机价格的map 表
map<int,double> flavor_cost;


int *Template;
int Template_size;

//遗传算法使用的种群大小，选择概率列表，适应度数组
double *possbile_list;
int pop_size;
double *fit_value_list;
double all_profit;
//设置一个解的动态数组全局变量
pop_individual * pop_list;


//对应的虚拟机资源消耗的约束
const int cpu_consume[24]={1,1,1,2,2,2,4,4,4,8,8,8,16,16,16,32,32,32,1,2,4,8,16,32};
const int mem_consume[24]={1,2,4,2,4,8,4,8,16,8,16,32,16,32,64,32,64,128,8,16,32,64,128,256};
/**************************************************************************************/

/**************************内部函数操作*****************************************************/
void supplement(pop_individual *X,const set<int>&ori_unplay_list);
void FindBestBoxIndex(int *worse_box_index_arr,int num,int X_index);
void FindWorseBoxIndex(int *worse_box_index_arr,int num,int X_index);
void SetPhysicalInfoArr(const limit_info *p,int size);
void ComputeAllProfit(const pre_flavor_info out_flavor_info);
void all_pop_list_init(pop_individual *p,int size);
int Min3Num(int a,int b,int c);
//设置回塞函数将箱子尽量塞满
//arr_size == limit_type_num
Result AddMoreBox(Result *r,pre_flavor_info out_flavor_info,int *Add_flovar_arr,int arr_size);

int Min3Num(int a,int b,int c)
{
    int Min=a;
    if(Min>b){
        Min=b;
    }
    if(Min>c){
        Min=c;
    }
    return Min;
}

/*********************************STL的操作*************************************************/
typedef pair<int, double> PAIR;

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
/***********************************************************************************/
//约束条件生成，嵌入的是ｉ+1
void SelectMustList()
{
    for (int i = 0; i <limit_type_num ; ++i) {
        if(cpu_consume[i]>=16 && cpu_consume[i]==mem_consume[i]){
            H_must_list.push_back(i+1);
        }else if(mem_consume[i]>=32 &&(mem_consume[i]/cpu_consume[i])>=4){
            L_must_list.push_back(i+1);
        }
    }
}

/**************************显示测试函数***************************/
void testprint(int size)
{
    for (int i = 0; i <size ; ++i) {
        printf("possible_list[%d]:%6f\n",i,possbile_list[i]);
    }
}
/************************************************************/

//总函数对应的初始化
void init(int size)
{

    possbile_list=(double *)malloc(sizeof(double)*size);
    if (possbile_list== NULL){
        fprintf(stderr,"malloc for possbile_list is failed\n");
        exit(1);
    }
    for (int i = 0; i <size ; ++i) {
        possbile_list[i]=0.0;
    }

    fit_value_list=(double *)malloc(sizeof(double)*size);
    if(fit_value_list==NULL){
        fprintf(stderr,"malloc for fit value list is failed\n");
        exit(1);
    }
    for (int j = 0; j <size ; ++j) {
        fit_value_list[j]=0.0;
    }

    pop_size=size;
//    生成对应的约束条件
    SelectMustList();
//     初始化对应的物理机结构数组
    InitPhysicalInfoArr();
//    注意容器的元素清除
    H_must_list.clear();
    L_must_list.clear();
    select_list.clear();
    flavor_cost.clear();
    Template_index_list.clear();
}

//总函数的释放函数
void end()
{
    if(possbile_list!=NULL){
        free(possbile_list);
    }
    if(fit_value_list!=NULL){
        free(fit_value_list);
    }
    if(pop_list!=NULL){
        free(pop_list);
    }
    if(Template!= NULL){
        free(Template);
    }
    H_must_list.clear();
    L_must_list.clear();
    Template_index_list.clear();
    flavor_cost.clear();
    select_list.clear();
}


/***********************轮盘赌选择速**************************/
//产生０－１的均匀分布的随机数
double myrand()
{
    double randnum=rand()/(RAND_MAX+1.0);
    return randnum;
}
//根据适应度函数重构对应的选择概率列表
double *cumsum(const double *p_list,int size)
{
//    debug test
    if(size!=pop_size){
        fprintf(stderr,"size can not match!\n");
    }

    double sum=0.0;
    for (int j = 0; j < size; ++j) {
        sum+=p_list[j];
    }
//    compute fit possible
    double  temp=0.0;
    for (int i = 0; i <size ; ++i) {
        double t=fit_value_list[i]/sum;
        possbile_list[i]=temp;
        temp+=t;
    }

    return possbile_list;
}
//根据概率表，基于轮盘赌进行选择下标遗传
int Select(int size)
{
    int select_index;
    double rand_num=myrand();


    int low=0;
    int high=size-1;
    int ans=(low+high)/2;

    if(rand_num>=possbile_list[size-1]){

////        test print
//        fprintf(stdout,"ceshi num is %lf\n",rand_num);
//        fprintf(stdout,"ceshi index is %d\n",size-1);

        return size-1;
    }

//    二分快速遍历
    while((high-low)>1 &&high>low){
        if (possbile_list[ans]<rand_num)
        {
            low=ans;
            ans=(low+high)/2;
        }else{
            high=ans;
            ans=(low+high)/2;
        }
    }
    select_index=low;

////    test debug
//    fprintf(stdout,"ceshi num is %lf\n",rand_num);
//    fprintf(stdout,"ceshi index is %d\n",select_index);

    return select_index;
}
/**********************************************************/

/**********************遍历函数*********************************/
//找到数组位置上为－１的数组的空闲位置
int find_free_pos(const int *a,int size)
{
    int index=-1;
    for (int i = 0; i <size ; ++i) {
        if(a[i]==-1){
            index=i;
            break;
        }
    }
    return index;
}

//匹配对应的数组中存在的数据，不存在返回－１
int find_value_in_arr(const int* a,int a_size,int value)
{
    int index=-1;
    for (int i = 0; i < a_size; ++i) {
        if(a[i]==value){
            index=i;
            break;
        }
    }
    return index;

}
/***********************************************************/

/**********************结构体初始化*****************************/
void Physical_Node_Init(Physical_Node *p)
{
    (*p).use_flag=0;
    (*p).curr_price=0.0;
    (*p).mem_max=0;
    (*p).remain_mem=0;
    (*p).cpu_max=0;
    (*p).remain_cpu=0;
    (*p).Physical_ID=-1;
    (*p).curr_contain_num=0;
    memset((*p).contain_Template_Index,-1, sizeof(int)*PHYSICAL_MAX_CONTAIN);

}

void pop_individual_init(pop_individual *p)
{
    for(int i = 0; i <3 ; ++i) {
        (*p).group_count_list[i]=0;
    }
    (*p).curr_physical_num=0;
    (*p).curr_cost=0.0;
    for (int j = 0; j < PHYSICAL_MAX_CONTAIN; ++j) {
        Physical_Node_Init(&(*p).Physical_play_list[j]);
    }

}

//种群结构体数组的初始化函数
void all_pop_list_init(pop_individual *p,int size)
{
    for (int i = 0; i <size ; ++i) {
        p[i].curr_cost = 0.0;
        p[i].curr_physical_num = 0;
        for (int j = 0; j < 3; ++j) {
            p[i].group_count_list[j] = 0;
        }
        for (int k = 0; k <PHYSICAL_MAX_CONTAIN ; ++k) {
            Physical_Node_Init(&p[i].Physical_play_list[k]);
        }

    }
}


void SetPhysicalInfoArr(const limit_info *p,int size)
{
    for (int i = 0; i <size ; ++i) {
        if((p+i)->pyhsical_Name=='G'){
            Physical_Info_Arr[0].pyhsical_Name='G';
            Physical_Info_Arr[0].cpu_value=(p+i)->cpu_value;
            Physical_Info_Arr[0].mem_value=(p+i)->mem_value;
            Physical_Info_Arr[0].money=(p+i)->money;
            select_list.push_back(0);
        }else if((p+i)->pyhsical_Name=='L'){
            Physical_Info_Arr[1].pyhsical_Name='L';
            Physical_Info_Arr[1].cpu_value=(p+i)->cpu_value;
            Physical_Info_Arr[1].mem_value=(p+i)->mem_value;
            Physical_Info_Arr[1].money=(p+i)->money;
            select_list.push_back(1);
        }else if((p+i)->pyhsical_Name=='H'){
            Physical_Info_Arr[2].pyhsical_Name='L';
            Physical_Info_Arr[2].cpu_value=(p+i)->cpu_value;
            Physical_Info_Arr[2].mem_value=(p+i)->mem_value;
            Physical_Info_Arr[2].money=(p+i)->money;
            select_list.push_back(2);
        } else{
            fprintf(stderr,"set physical info arr error\n");
            exit(1);
        }
    }
}



void InitPhysicalInfoArr()
{
    for (int i = 0; i <3 ; ++i) {
        Physical_Info_Arr[i].money=0.0;
        Physical_Info_Arr[i].mem_value=0;
        Physical_Info_Arr[i].cpu_value=0;
        Physical_Info_Arr[i].pyhsical_Name='X';
    }
}


//根据外部参数out_flavor_info，设计对应的Template和对应的flavor_cost
void SetTemplateAndFlavorCost(const pre_flavor_info out_flavor_info)
{
    int all_flavor_num=0;
    int i,j;
    all_profit=0.0;
    for( i = 0; i <limit_type_num ; ++i) {
        if(out_flavor_info.flavor_num[i]!=-1){
            all_flavor_num+=out_flavor_info.flavor_num[i];
//            同时设置全局变量的map-->flavor_cost
            flavor_cost[i+1]=out_flavor_info.flavor_cost[i];
            all_profit+=out_flavor_info.flavor_num[i]*out_flavor_info.flavor_cost[i];
        }
    }
    Template_size=all_flavor_num;
    Template=(int *)malloc(sizeof(int)*all_flavor_num);
    if(Template== NULL){
        fprintf(stderr,"malloc for Template is failed\n");
        exit(1);
    }

    int offset=0;
    for ( i = 0; i <limit_type_num; ++i) {
        int flavor_type=i+1;
        for ( j = 0; j < out_flavor_info.flavor_num[i]; ++j) {
            Template[offset]=flavor_type;
            Template_index_list.push_back(offset);
            offset++;
        }
    }

}

/***********************************************************/



/*******************遗传算法种群初始化**************/
//初始化种群的采用的是ＦＦ算法
pop_individual myffd(const int *a,int a_size)
{
    pop_individual ans;
//    init pop_individual
    pop_individual_init(&ans);


    int All_Count=0;
//   shuffle index
    random_shuffle(Template_index_list.begin(),Template_index_list.end());
    vector<int>::iterator it;
    for ( it = Template_index_list.begin(); it!=Template_index_list.end() ; it++)
    {
        int Template_index=*it;
        int play_flavor_type=a[Template_index];
        int flag=0;

        //L-1,0-G,2-H
        int belong_flag=0;
//      find play_flavor_type belong to?
        vector<int>::iterator L_it;
        L_it=find(L_must_list.begin(),L_must_list.end(),play_flavor_type);
        if (L_it!=L_must_list.end()){
            belong_flag=1;
        }else{
            vector<int>::iterator H_it;
            H_it=find(H_must_list.begin(),H_must_list.end(),play_flavor_type);
            if(H_it!=H_must_list.end()){
                belong_flag=2;
            }
        }
//      find free pos in curr physical computer
        if (belong_flag==0){
            for (int ser_num = 0; ser_num < All_Count; ++ser_num) {
                if(ans.Physical_play_list[ser_num].remain_cpu-cpu_consume[play_flavor_type-1]>=0 &&
                   ans.Physical_play_list[ser_num].remain_mem-mem_consume[play_flavor_type-1]>=0){
                    ans.Physical_play_list[ser_num].remain_cpu-=cpu_consume[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].remain_mem-=mem_consume[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].curr_price+=flavor_cost[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].curr_contain_num++;
                    int free_pos=-1;
                    free_pos=find_free_pos(ans.Physical_play_list[ser_num].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
                    if(free_pos==-1){
                        fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",ser_num);
                        exit(1);
                    }
                    ans.Physical_play_list[ser_num].contain_Template_Index[free_pos]=Template_index;
                    flag=1;
                    break;
                }
            }
        }else if(belong_flag==1){
            for (int ser_num = 0; ser_num <All_Count ; ++ser_num) {
                if(ans.Physical_play_list[ser_num].Physical_ID==1 &&
                   ans.Physical_play_list[ser_num].remain_cpu-cpu_consume[play_flavor_type-1]>=0&&
                   ans.Physical_play_list[ser_num].remain_mem-mem_consume[play_flavor_type-1]>=0){
                    ans.Physical_play_list[ser_num].remain_cpu-=cpu_consume[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].remain_mem-=mem_consume[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].curr_price+=flavor_cost[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].curr_contain_num++;
                    int free_pos=-1;
                    free_pos=find_free_pos(ans.Physical_play_list[ser_num].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
                    if(free_pos==-1){
                        fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",ser_num);
                        exit(1);
                    }
                    ans.Physical_play_list[ser_num].contain_Template_Index[free_pos]=Template_index;
                    flag=1;
                    break;
                }
            }
        }else if(belong_flag==2){
            for (int ser_num = 0; ser_num <All_Count ; ++ser_num) {
                if(ans.Physical_play_list[ser_num].Physical_ID==2 &&
                   ans.Physical_play_list[ser_num].remain_cpu-cpu_consume[play_flavor_type-1]>=0&&
                   ans.Physical_play_list[ser_num].remain_mem-mem_consume[play_flavor_type-1]>=0){
                    ans.Physical_play_list[ser_num].remain_cpu-=cpu_consume[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].remain_mem-=mem_consume[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].curr_price+=flavor_cost[play_flavor_type-1];
                    ans.Physical_play_list[ser_num].curr_contain_num++;
                    int free_pos=-1;
                    free_pos=find_free_pos(ans.Physical_play_list[ser_num].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
                    if(free_pos==-1){
                        fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",ser_num);
                        exit(1);
                    }
                    ans.Physical_play_list[ser_num].contain_Template_Index[free_pos]=Template_index;
                    flag=1;
                    break;
                }
            }

        }
//       if curr physical box can not play new physical box
        if (flag==0){
            int rand_select=-1;
            if(belong_flag==0){
//                rand select
                while(1){
                    rand_select=rand()%3;
                    vector<int>::iterator iter1;
                    iter1=find(select_list.begin(),select_list.end(),rand_select);
                    if(iter1!=select_list.end()) break;
                }

            }else if(belong_flag==1){
                rand_select=1;
            }else if(belong_flag==2){
                rand_select=2;
            }
            ans.Physical_play_list[All_Count].Physical_ID=rand_select;
            ans.Physical_play_list[All_Count].cpu_max=Physical_Info_Arr[rand_select].cpu_value;
            ans.Physical_play_list[All_Count].mem_max=Physical_Info_Arr[rand_select].mem_value;
            ans.Physical_play_list[All_Count].use_flag=1;
            ans.Physical_play_list[All_Count].remain_cpu=Physical_Info_Arr[rand_select].cpu_value-cpu_consume[play_flavor_type-1];
            ans.Physical_play_list[All_Count].remain_mem=Physical_Info_Arr[rand_select].mem_value-mem_consume[play_flavor_type-1];
            ans.Physical_play_list[All_Count].curr_price+=flavor_cost[play_flavor_type-1];
            ans.Physical_play_list[All_Count].curr_contain_num++;
            ans.group_count_list[rand_select]++;
            ans.curr_cost+=Physical_Info_Arr[rand_select].money;
            ans.curr_physical_num++;
            int free_pos=-1;
            free_pos=find_free_pos(ans.Physical_play_list[All_Count].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
            if(free_pos==-1){
                fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",All_Count);
                exit(1);
            }
            ans.Physical_play_list[All_Count].contain_Template_Index[free_pos]=Template_index;
            All_Count++;
        }


    }
    return ans;
}
//初始化种群
void POP_Init(int POP_size,const int *Template,int Template_size)
{
    pop_list=(pop_individual *)malloc(sizeof(pop_individual)*POP_size);
    if (pop_list != NULL) {
        all_pop_list_init(pop_list,POP_size);
        for (int i = 0; i < POP_size; ++i) {
            pop_individual ans = myffd(Template,Template_size);
            pop_list[i]=ans;
        }
    } else {
        fprintf(stderr, "malloc for pop_list is failed\n");
        exit(1);
    }
}

/***********************************************************/

void FindWorseBoxIndex(int *worse_box_index_arr,int num,int X_index)
{
    pop_individual temp=pop_list[X_index];
    int all_box_num=temp.curr_physical_num;
    map<int ,double> ave_profit;
    int i,j;
    for ( i = 0,j = 0; i <MAX_PHYSICAL_NUM && j<all_box_num; ++i) {
        Physical_Node t_node=temp.Physical_play_list[i];
        if(t_node.use_flag==1){
            int Physical_ID=t_node.Physical_ID;
            int use_cpu=t_node.cpu_max-t_node.remain_cpu;
            int use_mem=t_node.mem_max-t_node.remain_mem;
            double Physical_money=Physical_Info_Arr[Physical_ID].money;
            j++;
            double ave_cpu_cost=Physical_money/use_cpu;
            double ave_mem_cost=Physical_money/use_mem;
            ave_profit[i]=ave_cpu_cost;
        }
    }
    vector<PAIR>Index_ave_profit_vec(ave_profit.begin(),ave_profit.end());
    sort(Index_ave_profit_vec.begin(),Index_ave_profit_vec.end(),CmpByValueDescend());
    for(i =0; i<Index_ave_profit_vec.size() && i<num;i++){
        worse_box_index_arr[i]=Index_ave_profit_vec[i].first;
    }
    ave_profit.clear();
    Index_ave_profit_vec.clear();
}

void FindBestBoxIndex(int *best_box_index_arr,int num,int X_index)
{
    pop_individual temp=pop_list[X_index];
    int all_box_num=temp.curr_physical_num;
    map<int,double> ave_profit;
    int i,j;
    for ( i = 0,j=0; i <MAX_PHYSICAL_NUM &&j<all_box_num; ++i) {
        Physical_Node t_node=temp.Physical_play_list[i];
        if(t_node.use_flag==1){
            int Physical_ID=t_node.Physical_ID;
            int use_cpu=t_node.cpu_max-t_node.remain_cpu;
            int use_mem=t_node.mem_max-t_node.remain_mem;
            double Physical_money=Physical_Info_Arr[Physical_ID].money;
            j++;
            double ave_cpu_cost=Physical_money/use_cpu;
            double ave_mem_cost=Physical_money/use_mem;
            ave_profit[i]=ave_cpu_cost;
        }
    }
    vector<PAIR> Index_ave_profit_vec(ave_profit.begin(),ave_profit.end());
    sort(Index_ave_profit_vec.begin(),Index_ave_profit_vec.end(),CmpByValueAscend());
    for (i = 0; i <Index_ave_profit_vec.size() && i<num ; ++i) {
        best_box_index_arr[i]=Index_ave_profit_vec[i].first;
    }
    ave_profit.clear();
    Index_ave_profit_vec.clear();

}

/********************遗传算法的交叉*********************/
//交叉遗传ＭＲＣ
pop_individual MRC(int X_index,int Y_index,int Cross_Num)
{
    pop_individual GA_X;
//    init GA_X
    pop_individual_init(&GA_X);
//    find best box in X to remain
    int *best_box_index_arr;
    int i,j;
    best_box_index_arr = (int *)malloc(sizeof(int) * Cross_Num);
    if(best_box_index_arr==NULL){
        fprintf(stderr,"malloc for best_box_index_arr is failed\n");
        exit(1);
    }else{
        memset(best_box_index_arr,-1,sizeof(int)*Cross_Num);
    }
//    设置对应的逻辑unqiue序列
    set<int> unique_list;
    set<int> unplay_list;
//    针对比例选择箱子排序遗传
    FindBestBoxIndex(best_box_index_arr,Cross_Num,X_index);
    for(i=0;i<Cross_Num;i++){
        int Best_Index=best_box_index_arr[i];
        int Physical_ID=pop_list[X_index].Physical_play_list[Best_Index].Physical_ID;
        GA_X.group_count_list[Physical_ID]++;
        GA_X.Physical_play_list[i]=pop_list[X_index].Physical_play_list[Best_Index];
        GA_X.curr_physical_num++;
        double Physical_Cost=Physical_Info_Arr[Physical_ID].money;
        GA_X.curr_cost+=Physical_Cost;
//       依次遍历对应的已放置的序列
        int k;
        for (k=0, j = 0; j <PHYSICAL_MAX_CONTAIN && k< GA_X.Physical_play_list[i].curr_contain_num ; ++j) {
            if(GA_X.Physical_play_list[i].contain_Template_Index[j]!=-1){
                k++;
                int temp_Template_index=GA_X.Physical_play_list[i].contain_Template_Index[j];
                unique_list.insert(temp_Template_index);
            }
        }
    }
//      遍历集合查看需要重新放置的集合
    for (i = 0,j=0;i<MAX_PHYSICAL_NUM && j < pop_list[Y_index].curr_physical_num; ++i) {
        if(pop_list[Y_index].Physical_play_list[i].use_flag!=0){
//            针对已开箱子进行单个箱子检查
            j++;
            int k,g;
            int play_flag=0;
//            遍历该箱子中是否存在unqiue_list元素,如果存在则再下面进行将箱子未重复的元素倒入到unplay_list中
            set<int> temp_list;
            temp_list.clear();
            for ( g=0,k= 0; k < PHYSICAL_MAX_CONTAIN && g<pop_list[Y_index].Physical_play_list[i].curr_contain_num; ++k) {

                if(pop_list[Y_index].Physical_play_list[i].contain_Template_Index[k]!=-1){
                    int temp_Template_index=pop_list[Y_index].Physical_play_list[i].contain_Template_Index[k];
                    set<int>::iterator iter;
                    if((iter=find(unique_list.begin(),unique_list.end(),temp_Template_index))!=unique_list.end()){
//                        表示出现重复的索引标，该箱子需要重新放置
                        play_flag=1;
                    }else{
//                        非重复的索引放置
                        temp_list.insert(temp_Template_index);
                    }
                }
            }
//            针对上述
            set<int> temp_unplay_list;
            temp_unplay_list.clear();
            if(play_flag==1){
                set_union(unplay_list.begin(),unplay_list.end(),temp_list.begin(),temp_list.end(),inserter(temp_unplay_list,temp_unplay_list.begin()));
                unplay_list.clear();
                unplay_list=temp_unplay_list;
            }else{
//                    针对没有出现重复的放置的喜箱子进行保留
                int curr_physical_num=GA_X.curr_physical_num;
                GA_X.Physical_play_list[curr_physical_num]=pop_list[Y_index].Physical_play_list[i];
                int physical_ID=pop_list[Y_index].Physical_play_list[i].Physical_ID;
                GA_X.group_count_list[physical_ID]++;
                GA_X.curr_cost+=Physical_Info_Arr[physical_ID].money;
                GA_X.curr_physical_num++;
            }

        }
    }

//        这里插入回填算法设置
    supplement(&GA_X,unplay_list);


    unique_list.clear();
    unplay_list.clear();

    return GA_X;
}

/********************遗传算法的变异*********************/
//选择变异函数MRM
pop_individual MRM(int X_index,int Varition_num)
{
    pop_individual GA_X;
//    init GA_X
    pop_individual_init(&GA_X);
//    find worse box in X to replay
    int *worse_box_index_arr;
    int i,j;
    worse_box_index_arr = (int *)malloc(sizeof(int) * Varition_num);
    if(worse_box_index_arr==NULL){
        fprintf(stderr,"malloc for worse_box_index_arr is failed\n");
        exit(1);
    } else{
        memset(worse_box_index_arr,-1, sizeof(int)*Varition_num);
    }

    set<int> unplay_list;
    FindWorseBoxIndex(worse_box_index_arr,Varition_num,X_index);
//   拷贝已经没有被剔除的箱子保留
    int All_Count=0;
    for ( i = 0,j=0; i < MAX_PHYSICAL_NUM &&j<pop_list[X_index].curr_physical_num; ++i) {
        if(pop_list[X_index].Physical_play_list[i].use_flag!=0){
            j++;
            if(find_value_in_arr(worse_box_index_arr,Varition_num,i)==-1){
//                保留箱子
                int Physical_ID=pop_list[X_index].Physical_play_list[i].Physical_ID;
                GA_X.group_count_list[Physical_ID]++;
                GA_X.curr_physical_num++;
                GA_X.Physical_play_list[All_Count]=pop_list[X_index].Physical_play_list[i];
                All_Count++;
                GA_X.curr_cost+=Physical_Info_Arr[Physical_ID].money;
            }else{
//                删除箱子
                int m,n;
                for (int m = 0,n=0; m <PHYSICAL_MAX_CONTAIN && n<pop_list[X_index].Physical_play_list[i].curr_contain_num ; ++m) {
                    int temp_index=pop_list[X_index].Physical_play_list[i].contain_Template_Index[m];
                    if(temp_index!=-1){
                        n++;
                        unplay_list.insert(temp_index);
                    }
                }
            }
        }
    }
//   将根据回填函数,将未放置的箱子放回
    supplement(&GA_X,unplay_list);
    unplay_list.clear();

    return GA_X;
}


/*********************遗传算法的变异交叉的回填函数*************************/
void supplement(pop_individual *X,const set<int> &ori_unplay_list)
{
    int All_Count=(*X).curr_physical_num;
    vector<int> temp_unplay_list(ori_unplay_list.begin(),ori_unplay_list.end());
    random_shuffle(temp_unplay_list.begin(),temp_unplay_list.end());
    vector<int>::iterator unplay_list_iter;
    for(unplay_list_iter=temp_unplay_list.begin(); unplay_list_iter!=temp_unplay_list.end(); unplay_list_iter++)
    {
        int template_index=* unplay_list_iter;
        int unplay_flavor_type=Template[template_index];
        int flag=0;
        //L-1,0-G,2-H
        int belong_flag=0;
        vector<int>::iterator L_it;
        L_it=find(L_must_list.begin(),L_must_list.end(),unplay_flavor_type);
        if (L_it!=L_must_list.end()){
            belong_flag=1;
        }else{
            vector<int>::iterator H_it;
            H_it=find(H_must_list.begin(),H_must_list.end(),unplay_flavor_type);
            if(H_it!=H_must_list.end()){
                belong_flag=2;
            }
        }
//        find free pos in curr physical computer
        if(belong_flag==0){
            for (int ser_num = 0; ser_num <All_Count ; ++ser_num) {
                if((*X).Physical_play_list[ser_num].remain_cpu-cpu_consume[unplay_flavor_type-1]>=0&&
                   (*X).Physical_play_list[ser_num].remain_mem-mem_consume[unplay_flavor_type-1]>=0)
                {
                    (*X).Physical_play_list[ser_num].remain_cpu-=cpu_consume[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].remain_mem-=mem_consume[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].curr_price+=flavor_cost[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].curr_contain_num++;
                    int free_pos=-1;
                    free_pos=find_free_pos((*X).Physical_play_list[ser_num].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
                    if(free_pos==-1){
                        fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",ser_num);
                        exit(1);
                    }
                    (*X).Physical_play_list[ser_num].contain_Template_Index[free_pos]=template_index;
                    flag=1;
                    break;
                }
            }
        }else if(belong_flag==1){

            for (int ser_num = 0; ser_num <All_Count ; ++ser_num) {
                if((*X).Physical_play_list[ser_num].remain_cpu-cpu_consume[unplay_flavor_type-1]>=0&&
                   (*X).Physical_play_list[ser_num].remain_mem-mem_consume[unplay_flavor_type-1]>=0&&
                   (*X).Physical_play_list[ser_num].Physical_ID==1)
                {
                    (*X).Physical_play_list[ser_num].remain_cpu-=cpu_consume[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].remain_mem-=mem_consume[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].curr_price+=flavor_cost[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].curr_contain_num++;
                    int free_pos=-1;
                    free_pos=find_free_pos((*X).Physical_play_list[ser_num].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
                    if(free_pos==-1){
                        fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",ser_num);
                        exit(1);
                    }
                    (*X).Physical_play_list[ser_num].contain_Template_Index[free_pos]=template_index;
                    flag=1;
                    break;
                }
            }

        }else if(belong_flag == 2){

            for (int ser_num = 0; ser_num <All_Count ; ++ser_num) {
                if((*X).Physical_play_list[ser_num].remain_cpu-cpu_consume[unplay_flavor_type-1]>=0&&
                   (*X).Physical_play_list[ser_num].remain_mem-mem_consume[unplay_flavor_type-1]>=0&&
                   (*X).Physical_play_list[ser_num].Physical_ID==2)
                {
                    (*X).Physical_play_list[ser_num].remain_cpu-=cpu_consume[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].remain_mem-=mem_consume[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].curr_price+=flavor_cost[unplay_flavor_type-1];
                    (*X).Physical_play_list[ser_num].curr_contain_num++;
                    int free_pos=-1;
                    free_pos=find_free_pos((*X).Physical_play_list[ser_num].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
                    if(free_pos==-1){
                        fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",ser_num);
                        exit(1);
                    }
                    (*X).Physical_play_list[ser_num].contain_Template_Index[free_pos]=template_index;
                    flag=1;
                    break;
                }
            }
        }
//        if curr physical box can not play new physical box
        if (flag==0){
            int rand_select=-1;
            if(belong_flag==0){
//                rand select
                while(1){
                    rand_select=rand()%3;
                    vector<int>::iterator iter1;
                    iter1=find(select_list.begin(),select_list.end(),rand_select);
                    if(iter1!=select_list.end()) break;
                }

            }else if(belong_flag==1){
                rand_select=1;
            }else if(belong_flag==2){
                rand_select=2;
            }
            (*X).Physical_play_list[All_Count].Physical_ID=rand_select;
            (*X).Physical_play_list[All_Count].cpu_max=Physical_Info_Arr[rand_select].cpu_value;
            (*X).Physical_play_list[All_Count].mem_max=Physical_Info_Arr[rand_select].mem_value;
            (*X).Physical_play_list[All_Count].use_flag=1;
            (*X).Physical_play_list[All_Count].remain_cpu=Physical_Info_Arr[rand_select].cpu_value-cpu_consume[unplay_flavor_type-1];
            (*X).Physical_play_list[All_Count].remain_mem=Physical_Info_Arr[rand_select].mem_value-mem_consume[unplay_flavor_type-1];
            (*X).Physical_play_list[All_Count].curr_price+=flavor_cost[unplay_flavor_type-1];
            (*X).Physical_play_list[All_Count].curr_contain_num++;
            (*X).curr_cost+=Physical_Info_Arr[rand_select].money;
            (*X).group_count_list[rand_select]++;
            (*X).curr_physical_num++;
            int free_pos=-1;
            free_pos=find_free_pos((*X).Physical_play_list[All_Count].contain_Template_Index,PHYSICAL_MAX_CONTAIN);
            if(free_pos==-1){
                fprintf(stderr,"can not find free pos in Physical_play_list[%d]->contain_template_index\n",All_Count);
                exit(1);
            }
            (*X).Physical_play_list[All_Count].contain_Template_Index[free_pos]=template_index;
\
            All_Count++;
        }
//
    }
//end-for
    temp_unplay_list.clear();
}

/********************遗传算法的适应度函数**************************/
void ComputeAllProfit(const pre_flavor_info out_flavor_info)
{
    all_profit=0.0;
    for (int i = 0; i <limit_type_num ; ++i) {
        if(out_flavor_info.flavor_num[i]!=-1){
            all_profit+=out_flavor_info.flavor_num[i]*out_flavor_info.flavor_cost[i];
        }
    }
}

double git_fit_value(pop_individual p)
{
    double temp_fit_value=0.0;
    int i,j;
    double All_Physical_cost=0.0;

////    method1
//    for ( i = 0 ,j=0; i <MAX_PHYSICAL_NUM &&j<p.curr_physical_num; ++i) {
//        if(p.Physical_play_list[i].use_flag!=0){
//            j++;
//            int Physical_ID=p.Physical_play_list[i].Physical_ID;
//            All_Physical_cost+=Physical_Info_Arr[Physical_ID].money;
//        }
//    }
//    if(All_Physical_cost!=p.curr_cost){
//        fprintf(stderr,"physical_cost compute error\n");
//        exit(1);
//    }

//    method2
    for ( i = 0; i < 3; ++i) {
        All_Physical_cost+=p.group_count_list[i]*Physical_Info_Arr[i].money;
    }
    if(All_Physical_cost!=p.curr_cost){
        fprintf(stderr,"physical_cost compute error\n");
        exit(1);
    }
//    计算的目标函数公式
    temp_fit_value=(all_profit-All_Physical_cost*0.02)/all_profit;
    return temp_fit_value;
}


//设置回塞函数将箱子尽量塞满
//arr_size == limit_type_num
Result AddMoreBox(Result *r,pre_flavor_info out_flavor_info,int *Add_flovar_arr,int arr_size)
{

    memset(Add_flovar_arr,0,sizeof(int)*arr_size);
    vector<int> G_play_type;
    vector<int> H_play_type;
    vector<int> L_play_type;
    H_play_type.clear();
    G_play_type.clear();
    L_play_type.clear();
    for (int i = 0; i <limit_type_num ; ++i) {
        if(out_flavor_info.flavor_num[i]!=-1){
            vector<int>::iterator L_iter;
            L_iter=find(L_must_list.begin(),L_must_list.end(),i+1);
            vector<int>::iterator H_iter;
            H_iter=find(H_must_list.begin(),H_must_list.end(),i+1);
//            针对非必须约束的机型，则Ｇ，Ｈ，Ｌ都可以进行放置
            if(L_iter==L_must_list.end()&&H_iter==H_must_list.end()){
                G_play_type.push_back(i+1);
                H_play_type.push_back(i+1);
                L_play_type.push_back(i+1);
            }
//            针对必须的机型只能放置到对应的Ｌ集合
            if(L_iter!=L_must_list.end()){
                L_play_type.push_back(i+1);
            }
//            针对必须的机型必须放置到对应的Ｈ集合
            if(H_iter!=H_must_list.end()){
                H_play_type.push_back(i+1);
            }

        }
    }
//    遍历对应的ｒ结果处理塞满的情况
    int G_Num=(*r).G_Need_Num;
    if(G_Num>0){
        for (int count = 0; count < G_Num; ++count) {
            int remain_cpu=Physical_Info_Arr[0].cpu_value;
            int remain_mem=Physical_Info_Arr[0].mem_value;
            for (int i = 0; i <limit_type_num ; ++i) {
                int num=(*r).G_Need_list[count].contain_flavor_type_num[i];
                if(num>0){
                    remain_cpu-=num*cpu_consume[i];
                    remain_mem-=num*mem_consume[i];
                }
            }
//            如果一个资源放慢不需要执行遍历
            if(remain_cpu<=0 || remain_mem<=0){
                continue;
            }

            random_shuffle(G_play_type.begin(),G_play_type.end());
            vector<int>::iterator G_iter;
            for(G_iter=G_play_type.begin();G_iter!=G_play_type.end();G_iter++){
                int play_flavor_type=*G_iter;
                int cpu_limit_num=remain_cpu/cpu_consume[play_flavor_type-1];
                int mem_limit_num=remain_mem/mem_consume[play_flavor_type-1];
                int numb=Min3Num(cpu_limit_num,mem_limit_num,5);
                if(numb<=0)
                    continue;
                remain_cpu-=numb*cpu_consume[play_flavor_type-1];
                remain_mem-=numb*mem_consume[play_flavor_type-1];
                (*r).G_Need_list[count].contain_flavor_type_num[play_flavor_type-1]=+numb;
                Add_flovar_arr[play_flavor_type-1]+=numb;
            }

        }
    }

//  处理Ｌ
    int L_Num=(*r).L_Need_Num;
    if(L_Num>0){
        for (int count = 0; count < L_Num; ++count) {
            int remain_cpu=Physical_Info_Arr[1].cpu_value;
            int remain_mem=Physical_Info_Arr[1].mem_value;
            for (int i = 0; i <limit_type_num ; ++i) {
                int num=(*r).L_Need_list[count].contain_flavor_type_num[i];
                if(num>0){
                    remain_cpu-=num*cpu_consume[i];
                    remain_mem-=num*mem_consume[i];
                }
            }
//            如果一个资源放慢不需要执行遍历
            if(remain_cpu<=0 || remain_mem<=0){
                continue;
            }

            random_shuffle(L_play_type.begin(),L_play_type.end());
            vector<int>::iterator L_iter;
            for(L_iter=L_play_type.begin();L_iter!=L_play_type.end();L_iter++){
                int play_flavor_type=*L_iter;
                int cpu_limit_num=remain_cpu/cpu_consume[play_flavor_type-1];
                int mem_limit_num=remain_mem/mem_consume[play_flavor_type-1];
                int numb=Min3Num(cpu_limit_num,mem_limit_num,5);
                if(numb<=0)
                    continue;
                remain_cpu-=numb*cpu_consume[play_flavor_type-1];
                remain_mem-=numb*mem_consume[play_flavor_type-1];
                (*r).L_Need_list[count].contain_flavor_type_num[play_flavor_type-1]=+numb;
                Add_flovar_arr[play_flavor_type-1]+=numb;
            }

        }

    }
//  处理Ｈ塞满
    int H_Num=(*r).H_Need_Num;
    if(H_Num>0){
        for (int count = 0; count < H_Num; ++count) {
            int remain_cpu=Physical_Info_Arr[2].cpu_value;
            int remain_mem=Physical_Info_Arr[2].mem_value;
            for (int i = 0; i <limit_type_num ; ++i) {
                int num=(*r).H_Need_list[count].contain_flavor_type_num[i];
                if(num>0){
                    remain_cpu-=num*cpu_consume[i];
                    remain_mem-=num*mem_consume[i];
                }
            }
//            如果一个资源放慢不需要执行遍历
            if(remain_cpu<=0 || remain_mem<=0){
                continue;
            }

            random_shuffle(H_play_type.begin(),H_play_type.end());
            vector<int>::iterator H_iter;
            for(H_iter=H_play_type.begin();H_iter!=H_play_type.end();H_iter++){
                int play_flavor_type=*H_iter;
                int cpu_limit_num=remain_cpu/cpu_consume[play_flavor_type-1];
                int mem_limit_num=remain_mem/mem_consume[play_flavor_type-1];
                int numb=Min3Num(cpu_limit_num,mem_limit_num,5);
                if(numb<=0)
                    continue;
                remain_cpu-=numb*cpu_consume[play_flavor_type-1];
                remain_mem-=numb*mem_consume[play_flavor_type-1];
                (*r).H_Need_list[count].contain_flavor_type_num[play_flavor_type-1]=+numb;
                Add_flovar_arr[play_flavor_type-1]+=numb;
            }

        }

    }


    return (*r);
}

/************************遗传算法的主函数****************************/
Result GAA_main(int *pre_flavor_arr,int pre_flavor_arr_size,const pre_flavor_info flavor_info,const limit_info *physical_info,int physical_info_size ,const int size,const int max_iter,const double Cross_rate,
                const double Varition_rate,const double C_ratio,const double D_ratio)
{
//    根据外部输入的数据，进行设置


    int curr_iter;
    int i,j,m,n;
    init(size);

    InitPhysicalInfoArr();
//    根据外部的out_flavor_info 设置Template ,flavor_cost
    SetTemplateAndFlavorCost(flavor_info);
    //    设置物理机的参数
    SetPhysicalInfoArr(physical_info,physical_info_size);


//    初始化种群
    POP_Init(size,Template,Template_size);
//    计算个体的适应度值
    for (int i = 0; i <size ; ++i) {
        double temp_fit_value=0.0;
        temp_fit_value=git_fit_value(pop_list[i]);
        fit_value_list[i]=temp_fit_value;
    }
//    生成概率选择列表
    cumsum(fit_value_list,size);

//  开始迭代循环
    for(curr_iter=0;curr_iter<max_iter;curr_iter++){

        int Cross_num=size*Cross_rate;
        int Varition_num=size*Varition_rate;

        map<int,double> pop_index_and_cost;
        pop_index_and_cost.clear();

        pop_individual *new_pop_list=(pop_individual *)malloc(sizeof(pop_individual)*(size+Cross_num+Varition_num));
        if(new_pop_list== NULL){
            fprintf(stderr,"malloc for new_pop_list is failed\n");
            exit(1);
        }
        int new_pop_size=size+Cross_num+Varition_num;
        all_pop_list_init(new_pop_list,new_pop_size);


//        保存父代信息
        int curr_new_pop_size=0;
        for(i=0;i<size;i++){
            new_pop_list[curr_new_pop_size]=pop_list[i];
            pop_index_and_cost[curr_new_pop_size]=pop_list[i].curr_cost;
            curr_new_pop_size++;
        }


//        子代进行交叉
        for ( i = 0; i <Cross_num ; ++i) {
            int father_index=Select(size);
            int mother_index=Select(size);
            int cross_box_num=pop_list[father_index].curr_physical_num*C_ratio;
            if(cross_box_num<=0){
                cross_box_num=1;
            }
            pop_individual temp_divid=MRC(father_index,mother_index,cross_box_num);
            new_pop_list[curr_new_pop_size]=temp_divid;
            pop_index_and_cost[curr_new_pop_size]=temp_divid.curr_cost;
            curr_new_pop_size++;
        }
//        子代进行变异
        for ( i = 0; i <Varition_num ; ++i) {
            int father_index=Select(size);
            int varition_box_num=pop_list[father_index].curr_physical_num*D_ratio;
            if(varition_box_num<=0){
                varition_box_num=1;
            }
            pop_individual temp_divid=MRM(father_index,varition_box_num);
            new_pop_list[curr_new_pop_size]=temp_divid;
            pop_index_and_cost[curr_new_pop_size]=temp_divid.curr_cost;
            curr_new_pop_size++;
        }
//        对父代和子代进行筛选,选择最优的前N个组成新的pop_list,最优的在前面;
        vector<PAIR> pop_index_and_cost_vec(pop_index_and_cost.begin(),pop_index_and_cost.end());
        sort(pop_index_and_cost_vec.begin(),pop_index_and_cost_vec.end(),CmpByValueAscend());
        for ( i = 0; i < pop_index_and_cost_vec.size()&&i<size; ++i) {
            int temp_pop_index=pop_index_and_cost_vec[i].first;
            pop_list[i]=new_pop_list[temp_pop_index];
        }



//       更新对应的适应度分配和概率选择possible_list
        //    计算个体的适应度值
        for ( i = 0; i <size ; ++i) {
            double temp_fit_value=0.0;
            temp_fit_value=git_fit_value(pop_list[i]);
            fit_value_list[i]=temp_fit_value;
        }
        //    生成概率选择列表
        cumsum(fit_value_list,size);

//        结尾统一释放归０
        pop_index_and_cost.clear();
        pop_index_and_cost_vec.clear();
        if(new_pop_list!= NULL){
            free(new_pop_list);
        }

//        test print
        cout<<"temp best profit ratio is"<<fit_value_list[0]<<endl;
    }


    /**************************结果输出解码****************************************/
    int best_answer_index=0;
    pop_individual best_divid=pop_list[best_answer_index];




    Result r;
//    初始化对应的r结构体
    r.G_Need_Num=0;
    r.H_Need_Num=0;
    r.L_Need_Num=0;
    memset(r.G_Need_list,0,sizeof(result_physical_node)*MAX_PHYSICAL_NUM);
    memset(r.H_Need_list,0,sizeof(result_physical_node)*MAX_PHYSICAL_NUM);
    memset(r.L_Need_list,0,sizeof(result_physical_node)*MAX_PHYSICAL_NUM);

//   根据最优的解转化为对应的输出解
    r.G_Need_Num=best_divid.group_count_list[0];
    r.L_Need_Num=best_divid.group_count_list[1];
    r.H_Need_Num=best_divid.group_count_list[2];
    int use_physical_num=best_divid.curr_physical_num;
    int G_index=0,H_index=0,L_index=0;
    for ( i = 0,j=0; i<MAX_PHYSICAL_NUM && j<use_physical_num; ++i) {

        if(best_divid.Physical_play_list[i].use_flag==1){
            j++;
            Physical_Node temp_physical_node=best_divid.Physical_play_list[i];

            int Physical_ID=best_divid.Physical_play_list[i].Physical_ID;
            result_physical_node *temp_p = NULL;
            if(Physical_ID==0){
                temp_p=&r.G_Need_list[G_index];
                G_index++;
            }else if(Physical_ID==1){
                temp_p=&r.L_Need_list[L_index];
                L_index++;
            }else{
                temp_p=&r.H_Need_list[H_index];
                H_index++;
            }
//          统计单个物理机放置虚拟机的情况
            int temp_contain_num=best_divid.Physical_play_list[i].curr_contain_num;
            for ( m = 0,n=0; m < PHYSICAL_MAX_CONTAIN && n<temp_contain_num ; ++m) {
                int temp_index=temp_physical_node.contain_Template_Index[m];
                    if(temp_index!=-1){
                        n++;
                        int play_flavor_type=Template[temp_index];
                        temp_p->contain_flavor_type_num[play_flavor_type-1]++;
                    }
            }
        }
    }


    //    test print;

    double find_cost=best_divid.curr_cost;
    double score=(all_profit-find_cost*0.02)/all_profit;
    cout<<"before change final best profit ratio is:"<<score<<endl;

    //    这里针对ｒ设置塞满函数
    int Add_flovar_Num[limit_type_num];
    memset(Add_flovar_Num,0,sizeof(int)*limit_type_num);
    AddMoreBox(&r,flavor_info,Add_flovar_Num,limit_type_num);
//    对预测的数据进行微调整
    for (int k = 0; k <limit_type_num&&k<pre_flavor_arr_size ; ++k) {
        if(Add_flovar_Num[k]>0){
            pre_flavor_arr[k]+=Add_flovar_Num[k];
        }

    }

//    free-memory
    end();


    return r;
}