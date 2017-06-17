#pragma once
#include <vector>
#include <map>

struct Route {
    int id;
    int new_id = -1;
    double start_long;
    double start_lat;
    double end_long;
    double end_lat;
    double r;
};

void do_pairing(std::map<int, std::map<int, double>> &candidates,
                const std::map<int, Route> &psg_new, const std::map<int, Route> &drv_new,
                const std::map<int, Route> &passengers, const std::map<int, Route> &drivers);

double get_rate(const Route &passenger, const Route &driver);