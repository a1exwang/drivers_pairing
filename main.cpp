#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <map>
#include <unordered_map>
#include "pairing.h"
#include <dlib/optimization/max_cost_assignment.h>
#include "munkres.h"
#include "km.h"
#include <chrono>
using namespace std;
#define dict unordered_map
struct Bigraph {
    dict<int, dict<int, double>> d2p, p2d;
};

void read_routes(dict<int, Route> &out, string file_name) {
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
    r.r = sqrt((r.start_long - r.end_long) * (r.start_long - r.end_long) + (r.start_lat - r.end_lat) * (r.start_lat - r.end_lat));
    out[r.id] = r;
  }
}

int construct_dmap(dict<int, dict<int, double>> &d2p,
                   dict<int, dict<int, double>> &p2d,
                   const dict<int, Route> &psg, const dict<int, Route> &drv) {
  int edges = 0;
  for (auto &item : psg) { // passengers
    auto psg_id = item.first;
    auto &r = item.second;
    vector<int> driver_ids;
    for (int driver_id = 0; driver_id < drv.size(); ++driver_id) {
      double rate = get_rate(psg.at(psg_id), drv.at(driver_id));
      if (rate >= 0.8) {
        driver_ids.push_back(driver_id);
      }
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
  // copy
  Bigraph g = _g;
  while (g.d2p.size() > 0) {
    auto did0 = g.d2p.begin()->first;
    vector<pair<int, bool>> stack;
    // true for driver, false for passenger
    stack.push_back({did0, true});
    Bigraph sub = {{}, {}};
    while (!stack.empty()) {
      // pop stack
      auto _p = stack[0];
      auto id = _p.first;
      auto is_driver = _p.second;
      stack.erase(stack.begin());
      auto col = is_driver ? g.d2p : g.p2d;
      if (col.find(id) == col.end())
        continue;
      for (auto _1 : col.at(id)) {
        int other_id = _1.first;
        double w = _1.second;
        int did = is_driver ? id : other_id;
        int pid = is_driver ? other_id : id;
        if (is_driver) {
          if (sub.p2d.find(other_id) == sub.p2d.end())
            stack.push_back({other_id, !is_driver});
        }
        else {
          if (sub.d2p.find(other_id) == sub.d2p.end())
            stack.push_back({other_id, !is_driver});
        }

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
    if (sub.d2p.size() > 0)
      subs.push_back(sub);
  }
  assert(g.p2d.size() == 0);
}

double c[10010][10010] = {0};

double solve_pairing(const Bigraph &g, function<double (int, int)> cb) {
  dict<int, int> pid_map, pid_rmap;
  dict<int, int> did_map, did_rmap;
  int n_psg = (int)g.p2d.size(), n_drv = (int)g.d2p.size();
  int N = max({n_psg, n_drv});

  for (uint32_t i = 0; i < N; ++i) {
    for (uint32_t j = 0; j < N; ++j) {
      c[i][j] = 0;
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
        c[pid_new][did_new] = w;
        pid_new++;
      }
      else {
        c[pid_map[pid_old]][did_new] = w;
      }
    }
    did_new++;
  }
  vector<int> assignment;
  double total = km(assignment, c, N);
  double b = 0;
  for (int pid1 = 0; pid1 < N; ++pid1) {
    if (c[pid1][assignment[pid1]] > 0 &&
        pid_rmap.find(pid1) != pid_rmap.end() &&
        did_rmap.find(assignment[pid1]) != did_rmap.end())
      b += cb(pid_rmap[pid1], did_rmap[assignment[pid1]]);
  }
  return total;
}

int main(int argc, const char **argv) {
  if (argc != 3)
    return 1;

  dict<int, Route> passengers, drivers;
  read_routes(passengers, argv[1]);
  read_routes(drivers, argv[2]);

  dict<int, Route> psg_new, drv_new;
  dict<int, dict<int, double>> d2p, p2d;
  int edge_count = construct_dmap(d2p, p2d, passengers, drivers);
  cout << "Double map done, edges = " << edge_count << endl;
  cout << "Size = " << p2d.size() << "x" << d2p.size() << endl;
  vector<Bigraph> subs;
  construct_connected_subgraph(subs, {d2p, p2d});

  double sum = 0;
  int count = 0;
  map<int, pair<int, double>> result;
  double total1 = 0;
  for (auto g : subs) {
    total1 += solve_pairing(g, [&sum, &result, &count, &passengers, &drivers](int p, int d) -> double {
        double r = get_rate(passengers[p], drivers[d]);
        if (!(0.8 <= r && r <= 1)) {
          assert(0.8 <= r && r <= 1);
        }
        sum += r;
        count++;
        if (count % 100 == 0)
          cout << count << " Paired" << endl;
        result[p].first = d;
        result[p].second = r;
        return r;
    });
  }
  ofstream of("result");
  for (auto _1 : result) {
    int p = _1.first, d = _1.second.first;
    double r = _1.second.second;
    of << p << ' ' << d << ' ' << r << endl;
  }
  cout << "Count = " << count << ", Sum = " << sum << ", Average = " << sum / count << endl;

  return 0;
}