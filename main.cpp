#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <map>
#include "pairing.h"
#include <dlib/optimization/max_cost_assignment.h>
#include "munkres.h"
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

  cout << "1 -> 1 pairs" << unique_pairs.size() << endl;
  cout << "1 -> n pairs" << valid_count << endl;
  cout << "Starting " << psg_new.size() << "x" << drv_new.size() << " Hungarians" << endl;
  Matrix<double> matrix(psg_new.size(), drv_new.size());

  for (uint32_t passenger_id = 0; passenger_id < psg_new.size(); ++passenger_id) {
    for (uint32_t driver_id = 0; driver_id < drv_new.size(); ++driver_id) {
      matrix(passenger_id, driver_id) = numeric_limits<double>::infinity();
    }
  }

  for (auto it = candidates.begin(); it != candidates.end(); it++) {
    int passenger_id = it->first;
    for (auto b : it->second) {
      auto driver_id = b.first;
      double val = b.second;
      assert (driver_id >= 0 && passenger_id >= 0 && driver_id < drv_new.size() && passenger_id < psg_new.size());
      matrix((uint32_t)passenger_id, (uint32_t)driver_id) = 1 - val;
    }
  }

  cout << "begin hungarian" << endl;
  Munkres<double> m;
  m.solve(matrix);
  double sum = 0;
  for ( int i = 0 ; i < psg_new.size() ; i++ ) {
    for ( int j = 0 ; j < drv_new.size() ;j++ ) {
      double val =  matrix((uint32_t)i, (uint32_t)j);
      if (val >= 0) {
        if (candidates[i].find(j) != candidates[i].end()) {
          int pid0 = psg_new.at(i).id;
          int did0 = drv_new.at(j).id;
          double rate = get_rate(psg_new.at(i), drv_new.at(j));
          cout << "(" << pid0 << ", " << did0 << ") = " << rate << std::endl;
          sum += rate;
        }
      }
    }
  }
  cout << "sum = " << sum << endl;
  std::cout << std::endl;
  return 0;
}