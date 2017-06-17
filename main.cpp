#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <map>
#include <functional>
#include "pairing.h"
using namespace std;

struct Route {
    uint64_t id;
    double start_long;
    double start_lat;
    double end_long;
    double end_lat;
    double r;
};

void get_point_range(
    double &long_min, double &lat_min, double &long_max, double &lat_max,
    const map<uint64_t, Route> &r) {
  long_min = 200;
  long_max = 0;
  lat_min = 200;
  lat_max = 0;
  for (auto &item: r) {
    auto &route = item.second;
    if (route.start_long > long_max)
      long_max = route.start_long;
    if (route.start_long < long_min)
      long_min = route.start_long;
    if (route.start_lat > lat_max)
      lat_max = route.start_lat;
    if (route.start_lat < lat_min)
      lat_min = route.start_lat;
    if (route.end_long > long_max)
      long_max = route.end_long;
    if (route.end_long < long_min)
      long_min = route.end_long;
    if (route.end_lat > lat_max)
      lat_max = route.end_lat;
    if (route.end_lat < lat_min)
      lat_min = route.end_lat;
  }
}

void get_grid_location(
    int &long_id, int &lat_id,
    double longitude, double latitude,
    double long_min, double lat_min, double long_max, double lat_max,
    int long_count, int lat_count) {

  long_id = (int)((longitude - long_min) / (long_max - long_min) * long_count);
  lat_id = (int)((latitude - lat_min) / (lat_max - lat_min) * lat_count);
}


constexpr int LONG_COUNT = 50;
constexpr int LAT_COUNT = 50;
void traverse_all_points_in_circle(
    map<uint64_t, bool> grids[LONG_COUNT][LAT_COUNT],
    double longitude, double latitude,
    double r,
    double long_min, double lat_min, double long_max, double lat_max,
    function<bool (uint64_t id, bool is_start)> cb) {

  int current_long_id, current_lat_id;
  get_grid_location(current_long_id, current_lat_id,
                    longitude, latitude,
                    long_min, lat_min, long_max, lat_max, LONG_COUNT, LAT_COUNT);

  double long_delta = (long_max - long_min) / LONG_COUNT;
  double lat_delta = (lat_max - lat_min) / LAT_COUNT;
  // calculate d1, d2
  double d1 = longitude - long_delta * current_long_id;
  double d2 = long_delta - d1;

  int min_long_id = current_long_id - (int)ceil(fmax(0, (r - d1)) / long_delta);
  int max_long_id = current_long_id + (int)ceil(fmax(0, (r - d2)) / long_delta);

  double d3 = latitude - lat_delta * current_lat_id;
  double d4 = lat_delta - d3;
  int min_lat_id = current_lat_id - (int)ceil(fmax(0, (r - d3)) / lat_delta);
  int max_lat_id = current_lat_id + (int)ceil(fmax(0, (r - d4)) / lat_delta);

  for (int i = max(0, min_long_id); i <= max_long_id && i < LONG_COUNT; ++i) {
    for (int j = max(0, min_lat_id); i <= max_lat_id && i < LAT_COUNT; ++i) {
      for (auto &g : grids[i][j]) {
        cb(g.first, g.second);
      }
    }
  }
}

void read_routes(map<uint64_t, Route> &out, string file_name) {
  ifstream f_drivers(file_name);
  uint32_t id = 0;
  while (!f_drivers.eof()) {
    Route r;
    string s;
    getline(f_drivers, s);
    if (s.size() == 0)
      continue;
    stringstream ss(s);
    string item;
    r.id = id; id++;//r.id = (uint64_t) atoll(item.c_str());
    getline(ss, item, ','); r.start_long = atof(item.c_str());
    getline(ss, item, ','); r.start_lat = atof(item.c_str());
    getline(ss, item, ','); r.end_long = atof(item.c_str());
    getline(ss, item, ','); r.end_lat = atof(item.c_str());
    r.r = sqrt((r.start_long - r.end_long) * (r.start_long - r.end_long) +
               (r.start_lat - r.end_lat) * (r.start_lat - r.end_lat));
    out[r.id] = r;
  }
}
int main(int argc, const char **argv) {
  if (argc != 3)
    return 1;

  map<uint64_t, Route> passengers, drivers;
  read_routes(passengers, argv[1]);
  read_routes(drivers, argv[2]);

  double long_min, long_max, lat_min, lat_max;
  get_point_range(long_min, lat_min, long_max, lat_max, drivers);

  map<uint64_t, bool> driver_grids[LONG_COUNT][LAT_COUNT];
  for (auto item : drivers) {
    auto &route = item.second;
    auto &id = item.first;
    int long_id, lat_id;
    get_grid_location(long_id, lat_id, route.start_long, route.start_lat,
                      long_min, lat_min, long_max, lat_max, LONG_COUNT, LAT_COUNT);
    if (driver_grids[long_id][lat_id].find(id) == driver_grids[long_id][lat_id].end()) {
      driver_grids[long_id][lat_id] = {};
    }
    driver_grids[long_id][lat_id][id] = true;
  }

  map<uint64_t, map<uint64_t, double>> candidates;

  double sum = 0;
  for (auto &item : passengers) { // passengers
    auto &r = item.second;
    auto passenger_id = item.first;
    int n = 0;
    int last = 0;
    int a = 0;
    traverse_all_points_in_circle(
        driver_grids,
        r.start_long, r.start_lat,
        r.r / 4,
        long_min, lat_min, long_max, lat_max, [&n, &last, passenger_id, &candidates, &drivers, &passengers](uint64_t driver_id, bool is_start) -> bool {
            double rate = passengers[passenger_id].r /
                    (passengers[passenger_id].r +
                        sqrt((drivers[driver_id].start_long - passengers[passenger_id].start_long) * (drivers[driver_id].start_long - passengers[passenger_id].start_long) +
                                 (drivers[driver_id].start_lat - passengers[passenger_id].start_lat) * (drivers[driver_id].start_lat - passengers[passenger_id].start_lat)) +
                        sqrt((drivers[driver_id].end_long - passengers[passenger_id].end_long) * (drivers[driver_id].end_long - passengers[passenger_id].end_long) +
                             (drivers[driver_id].end_lat - passengers[passenger_id].end_lat) * (drivers[driver_id].end_lat - passengers[passenger_id].end_lat))
                    );
            if (rate >= 0.8) {
              if (candidates.find(passenger_id) == candidates.end()) {
                candidates[passenger_id] = {};
              }
              candidates[passenger_id][driver_id] = rate;
              n++;
              last = driver_id;
//            cout << "\t" << driver_id;
            }
            return true;
        });
    if (n > 0)
      cout << "Point " << passenger_id << ", " << last << " count = " << n << endl;
    sum += n;
  }
//  cout << "average " << sum / data.size() << endl;

  do_pairing(candidates, (uint32_t)passengers.size(), (uint32_t)drivers.size());


  return 0;
}