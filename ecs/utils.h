#ifndef ECS_UTILS_H
#define ECS_UTILS_H

#include <string>
#include <chrono>

using namespace std;

struct Date {
    int y, m, d;
};


#ifdef _DEBUG
void time_it(const char *info);
void time_clear();

#define TIME_IT time_it
#define TIME_CLEAR time_clear
#else
#define TIME_IT(...)
#define TIME_CLEAR(...)
#endif

int get_diff_days(Date bigger, Date smaller);
string trim_right(const string &str, const string &pattern);
#endif //ECS_UTILS_H
