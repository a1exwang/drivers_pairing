#include <cstdlib>
#include <vector>
#include <map>
#include <dlib/optimization/max_cost_assignment.h>

using namespace std;

void do_pairing(map<uint64_t, map<uint64_t, double>> candidates,
          uint32_t passenger_count,
          uint32_t driver_count) {
  dlib::matrix<uint32_t> matrix(passenger_count, driver_count);
  for (uint32_t passenger_id = 0; passenger_id < (int)passenger_count; ++passenger_id) {
    for (uint32_t driver_id = 0; driver_id < (int)driver_count; ++driver_id) {
      matrix(passenger_id, driver_id) = 0;
    }
  }

  for (auto a : candidates) {
    auto passenger_id = a.first;
    for (auto b : a.second) {
      auto driver_id = b.first;
      double val = b.second;
      matrix(passenger_id, driver_id) = (uint32_t)(std::numeric_limits<uint32_t>::max() * val);
    }
  }

  cout << "begin hungarian" << endl;
  auto assignment = dlib::max_cost_assignment(matrix);

  for (unsigned int i = 0; i < assignment.size(); i++) {
    if (candidates[i].find(assignment[i]) != candidates[i].end())
      cout << "(" << i << ", " << assignment[i] << ")" << std::endl;
  }

  std::cout << std::endl;
}

