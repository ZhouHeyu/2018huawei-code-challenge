#include "utils.h"

const int month_days[12] = {31, 28, 31, 30, 31, 30,
                            31, 31, 30, 31, 30, 31};

int count_leap_years(Date d) {
    int years = d.y;

    if (d.m <= 2)
        years--;

    return years / 4 - years / 100 + years / 400;
}

int get_diff_days(Date bigger, Date smaller) {
    long int n1 = smaller.y * 365 + smaller.d;

    for (int i = 0; i < smaller.m - 1; i++)
        n1 += month_days[i];
    n1 += count_leap_years(smaller);
    long int n2 = bigger.y * 365 + bigger.d;
    for (int i = 0; i < bigger.m - 1; i++)
        n2 += month_days[i];
    n2 += count_leap_years(bigger);
    return (n2 - n1);
}

string trim_right(const string &str, const string &pattern) {
    return str.substr(0, str.find_last_not_of(pattern) + 1);
}

#ifdef _DEBUG
auto _timer = chrono::steady_clock::now();
void time_it(const char *info) {
    auto ms = (chrono::steady_clock::now() - _timer)/1000000 ;
    _timer = chrono::steady_clock::now();
    printf("\n\e[31m>>>>> [ %s ] : %ld <<<<<\e[0m\n", info, ms);
}
void time_clear(){
    _timer = chrono::steady_clock::now();
}
#endif
