#include "predict.h"
#include "preprocess.h"
#include "GGA.h"

//TODO:
void result_to_string(Result result) {

}

void predict_server(char *info[MAX_INFO_NUM], int info_line_num,
                    char *data[MAX_DATA_NUM], int data_num, char *filename) {

    init_global_vars();
    parse_input(info, info_line_num);
    parse_data(data, data_num);
//    dump_history_to_file();
    char *result_file = (char *) "17\n\n0 8 0 20";
    write_result(result_file, filename);
}
