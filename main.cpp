#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <map>
#include "pairing.h"
#include <dlib/optimization/max_cost_assignment.h>
using namespace std;

void read_routes(map<int, Route> &out, string file_name) {
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
    r.id = id; id++;
    ss >> item; r.start_long = atof(item.c_str());
    ss >> item; r.start_lat = atof(item.c_str());
    ss >> item; r.end_long = atof(item.c_str());
    ss >> item; r.end_lat = atof(item.c_str());
    r.r = sqrt((r.start_long - r.end_long) * (r.start_long - r.end_long) +
               (r.start_lat - r.end_lat) * (r.start_lat - r.end_lat));
    out[r.id] = r;
  }
}

int main(int argc, const char **argv) {
  dlib::matrix<uint32_t> matrix(55, 98);
  dlib::max_cost_assignment(matrix);

  if (argc != 3)
    return 1;

  map<int, Route> passengers, drivers;
  read_routes(passengers, argv[1]);
  read_routes(drivers, argv[2]);

  int valid_count = 0;
  int pid_new = 0, did_new = 0;
  map<int, Route> psg_new, drv_new;
  map<int, map<int, double>> candidates;
  map<int, int> unique_pairs;
  for (auto &item : passengers) { // passengers
    auto passenger_id = item.first;
    auto &r = item.second;
    int n = 0;
    int outer_n = 0;
    vector<int> driver_ids;
    for (int driver_id = 0; driver_id < drivers.size(); ++driver_id) {
      double rate = get_rate(passengers[passenger_id], drivers[driver_id]);
      if (rate >= 0.8) {
        n++;
        driver_ids.push_back(driver_id);
      }
      outer_n++;
    }
    if (n > 0) {
      valid_count++;
      if (n == 1) {
        unique_pairs[passenger_id] = driver_ids[0];
      }
      else {
        passengers[passenger_id].new_id = pid_new;
        psg_new[pid_new] = passengers[passenger_id];
        for (auto driver_id : driver_ids) {
          if (drivers[driver_id].new_id == -1) {
            drivers[driver_id].new_id = did_new;
            drv_new[did_new] = drivers[driver_id];
            did_new++;
          }
          double rate = get_rate(passengers[passenger_id], drivers[driver_id]);
          candidates[pid_new][drivers[driver_id].new_id] = rate;
        }
        pid_new++;
      }
    }
  }

  cout << "Unique pairs " << unique_pairs.size() << ", Valid pairs " << valid_count << endl;
  do_pairing(candidates, psg_new, drv_new, passengers, drivers);
  return 0;
}