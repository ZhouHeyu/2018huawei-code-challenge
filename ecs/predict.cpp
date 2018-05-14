#include "predict.h"
#include "preprocess.h"
#include <sstream>

int vm_preds[24] = {1, 0, 0, 3, 12, 13}; //>=0

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
        if (vm_preds[i] > 0) {
            total_vm_pred += vm_preds[i];
        }
    }
    output << total_vm_pred << endl;
    for (int i = 0; i < MAX_FLAVOR_COUNT; ++i) {
        if (vm_preds[i] >= 0) {
            output << "flavor" << i + 1 << " " << vm_preds[i] << endl;
        }
    }
    output << endl;
    _write_phy_result(output, "General", result.G_Need_Num, result.G_Need_list);
    _write_phy_result(output, "Large-Memory", result.L_Need_Num, result.L_Need_list);
    _write_phy_result(output, "High-Performance", result.H_Need_Num, result.H_Need_list);
    write_result(output.str().data(), fname);
}

void predict_server(char *info[MAX_INFO_NUM], int info_line_num,
                    char *data[MAX_DATA_NUM], int data_num, char *filename) {

    init_global_vars();
    parse_input(info, info_line_num);
    parse_data(data, data_num);
//    dump_history_to_file();
    Result result = {2, 0, 3};
    result.H_Need_list[0].contain_flavor_type_num[0] = 1;
    result.H_Need_list[0].contain_flavor_type_num[1] = 2;
    result.H_Need_list[1].contain_flavor_type_num[1] = 3;
    write_result(result, filename);
}
