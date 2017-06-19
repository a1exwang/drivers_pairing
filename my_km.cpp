#include "km.h"
#include <stdio.h>
#include <random>
#include <ctime>


double c[10010][10010];
int main(int argc, char **argv) {
  srand(time(nullptr));
  vector<int> a;
  int n = atoi(argv[1]);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      c[i][j] = (double)rand() / RAND_MAX;
    }
  }
  km(a, c, n);
  return 0;
}
