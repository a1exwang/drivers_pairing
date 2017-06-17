#pragma once
#include <vector>
#include <map>
void do_pairing(std::map<uint64_t, std::map<uint64_t, double>> candidates,
          uint32_t passenger_count,
          uint32_t driver_count);
