#pragma once
#include <vector>
#include <map>

struct Route {
    int id;
    double start_long;
    double start_lat;
    double end_long;
    double end_lat;
    double r;
};

double get_rate(const Route &passenger, const Route &driver);