#ifndef ECS_UTILS_H
#define ECS_UTILS_H

struct Date {
    int y, m, d;
};

int get_diff_days(Date bigger, Date smaller);

#endif //ECS_UTILS_H
