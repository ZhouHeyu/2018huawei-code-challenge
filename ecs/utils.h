#ifndef ECS_UTILS_H
#define ECS_UTILS_H

#include <string>
using namespace std;

struct Date {
    int y, m, d;
};

int get_diff_days(Date bigger, Date smaller);
string trim_right(const string& str,const string &pattern);
#endif //ECS_UTILS_H
