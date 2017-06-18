#include <cstdlib>
#include <vector>
#include <map>
#include <dlib/optimization/max_cost_assignment.h>
#include "pairing.h"
#include <limits>
#include <iostream>
#include "munkres.h"


using namespace std;

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

