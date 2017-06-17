#include <cstdlib>
#include <vector>
#include <map>
#include <dlib/optimization/max_cost_assignment.h>
#include "pairing.h"

using namespace std;

void do_pairing(map<int, map<int, double>> &candidates,
                const map<int, Route> &psg_new, const map<int, Route> &drv_new,
                const std::map<int, Route> &_passengers0, const std::map<int, Route> &_drivers0) {
  cout << "Starting " << psg_new.size() << "x" << drv_new.size() << " Hungarians" << endl;
  dlib::matrix<uint32_t> matrix(psg_new.size(), drv_new.size());
  for (uint32_t passenger_id = 0; passenger_id < psg_new.size(); ++passenger_id) {
    for (uint32_t driver_id = 0; driver_id < drv_new.size(); ++driver_id) {
      matrix(passenger_id, driver_id) = 0;
    }
  }

  for (auto it = candidates.begin(); it != candidates.end(); it++) {
    int passenger_id = it->first;
    for (auto b : it->second) {
      auto driver_id = b.first;
      double val = b.second;
      assert (driver_id >= 0 && passenger_id >= 0 && driver_id < drv_new.size() && passenger_id < psg_new.size());
      matrix(passenger_id, driver_id) =
          (uint32_t)(std::numeric_limits<uint32_t>::max() * (val - 0.8));
    }
  }

  cout << "begin hungarian" << endl;
  auto assignment = dlib::max_cost_assignment(matrix);

  double sum = 0;
  for (unsigned int i = 0; i < assignment.size() && i < psg_new.size(); i++) {
    int j = (int)assignment[i];
    int pid0 = psg_new.at(i).id;
    int did0 = drv_new.at(j).id;

    if (candidates[i].find(j) != candidates[i].end()) {
      cout << "(" << pid0 << ", " << did0 << ")" << std::endl;
      sum += get_rate(psg_new.at(i), drv_new.at(j));
    }
  }

  std::cout << std::endl;
}

double get_rate(const Route &passenger, const Route &driver) {

  double rate = passenger.r /
                (passenger.r +
                 sqrt((driver.start_long - passenger.start_long) * (driver.start_long - passenger.start_long) +
                      (driver.start_lat - passenger.start_lat) * (driver.start_lat - passenger.start_lat)) +
                 sqrt((driver.end_long - passenger.end_long) * (driver.end_long - passenger.end_long) +
                      (driver.end_lat - passenger.end_lat) * (driver.end_lat - passenger.end_lat))
                );
  return rate;
}

