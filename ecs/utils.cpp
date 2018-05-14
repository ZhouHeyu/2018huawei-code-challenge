//
// Created by dgd on 18-5-14.
//

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
