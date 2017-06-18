#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include "pairing.h"
#include <dlib/optimization/max_cost_assignment.h>
#include "munkres.h"
using namespace std;
struct Bigraph {
    unordered_map<int, unordered_map<int, double>> d2p, p2d;
};

void read_routes(unordered_map<int, Route> &out, string file_name) {
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

int construct_dmap(unordered_map<int, unordered_map<int, double>> &d2p,
                   unordered_map<int, unordered_map<int, double>> &p2d,
                   const unordered_map<int, Route> &psg, const unordered_map<int, Route> &drv) {
  int edges = 0;
  for (auto &item : psg) { // passengers
    auto psg_id = item.first;
    auto &r = item.second;
    int outer_n = 0;
    vector<int> driver_ids;
    for (int driver_id = 0; driver_id < drv.size(); ++driver_id) {
      double rate = get_rate(psg.at(psg_id), drv.at(driver_id));
      if (rate >= 0.8) {
        driver_ids.push_back(driver_id);
      }
      outer_n++;
    }
    for (auto drv_id : driver_ids) {
      double rate = get_rate(psg.at(psg_id), drv.at(drv_id));
      d2p[drv_id][psg_id] = rate;
      p2d[psg_id][drv_id] = rate;
      edges++;
    }
  }
  return edges;
}

void construct_connected_subgraph(
    vector<Bigraph> &subs,
    const Bigraph &_g) {

  int edge_count = 0;
  Bigraph g = _g;
  while (g.d2p.size() > 0) {
    auto did0 = g.d2p.begin()->first;
    vector<pair<int, bool>> stack;
    // true for driver, false for passenger
    stack.push_back({did0, true});
    Bigraph sub;
    while (!stack.empty()) {
      // pop stack
      auto _p = stack[0];
      auto id = _p.first;
      auto is_driver = _p.second;
      stack.pop_back();

      auto col = is_driver ? g.d2p : g.p2d;
      if (col.find(id) == col.end())
        continue;
      for (auto _1 : col.at(id)) {
        int other_id = _1.first;
        double w = _1.second;
        int did = is_driver ? id : other_id;
        int pid = is_driver ? other_id : id;
        stack.push_back({other_id, !is_driver});

        // delete the edge in previous graph
        g.d2p[did].erase(pid);
        if (g.d2p[did].size() == 0)
          g.d2p.erase(did);
        g.p2d[pid].erase(did);
        if (g.p2d[pid].size() == 0)
          g.p2d.erase(pid);

        // add the edge to sub graph
        sub.d2p[did][pid] = w;
        sub.p2d[pid][did] = w;
        edge_count++;
        if (edge_count % 100 == 0)
          cout << edge_count << " edges processed" << endl;
      }
    }
    subs.push_back(sub);
  }
}

void solve_pairing(const Bigraph &g, function<void (int, int)> cb) {
  unordered_map<int, int> pid_map, pid_rmap;
  unordered_map<int, int> did_map, did_rmap;
  int n_psg = (int)g.p2d.size(), n_drv = (int)g.d2p.size();
  if (n_psg > 1) {
    cout << endl;
  }
  Matrix<double> matrix((uint32_t)n_psg, (uint32_t)n_drv);

  for (uint32_t passenger_id = 0; passenger_id < n_psg; ++passenger_id) {
    for (uint32_t driver_id = 0; driver_id < n_drv; ++driver_id) {
      matrix(passenger_id, driver_id) = numeric_limits<double>::infinity();
    }
  }

  int did_new = 0, pid_new = 0;

  for (auto _item : g.d2p) {
    auto did_old = _item.first;
    did_map[did_old] = did_new;
    did_rmap[did_new] = did_old;

    for (auto _1 : _item.second) {
      auto pid_old = _1.first;
      double w = _1.second;
      if (pid_map.find(pid_old) == pid_map.end()) {
        pid_map[pid_old] = pid_new;
        pid_rmap[pid_new] = pid_old;
        matrix((uint32_t)pid_new, (uint32_t)did_new) = w;
        pid_new++;
      }
      else {
        matrix((uint32_t)pid_map[pid_old], (uint32_t)did_new) = w;
      }
    }
    did_new++;
  }
  if (n_psg > 1 || n_drv > 1)
    cout << "begin hungarian " << n_psg << "x" << n_drv << endl;
  Munkres<double> m;
  m.solve(matrix);
  for ( int i = 0 ; i < n_psg ; i++ ) {
    for ( int j = 0 ; j < n_drv ;j++ ) {
      double val =  matrix((uint32_t)i, (uint32_t)j);
      if (val >= 0) {
        int pid_old = pid_rmap[i];
        int did_old = did_rmap[j];
        cb(pid_old, did_old);
      }
    }
  }
  std::cout << std::endl;
}

int main(int argc, const char **argv) {
  if (argc != 3)
    return 1;

  unordered_map<int, Route> passengers, drivers;
  read_routes(passengers, argv[1]);
  read_routes(drivers, argv[2]);

  unordered_map<int, Route> psg_new, drv_new;
  unordered_map<int, unordered_map<int, double>> d2p, p2d;
  int edge_count = construct_dmap(d2p, p2d, passengers, drivers);
  cout << "Double unordered_map done, edges = " << edge_count << endl;
  vector<Bigraph> subs;
  construct_connected_subgraph(subs, {d2p, p2d});
  int i = 0;

  double sum = 0;
  int count = 0;
  map<int, map<int, double>> result;
  for (auto g : subs) {
    solve_pairing(g, [&sum, &result, &count, &passengers, &drivers](int p, int d) -> void {
        cout << "(" << p << ", " << d << ")" << std::endl;
        double r = get_rate(passengers[p], drivers[d]);
        sum += r;
        count++;
        result[p][d] = r;
    });
  }
  double sum1 = 0;
  for (auto _1 : result) {
    for (auto _2 : _1.second) {
      sum1 += _2.second;
    }
  }
  cout << "Count = " << count << ", Sum = " << sum << ", Average = " << sum / count << endl;
  cout << "Count1 = " << result.size() << ", Sum = "<<  sum1 << ", Average = " << sum1 / result.size() << endl;

  return 0;
}