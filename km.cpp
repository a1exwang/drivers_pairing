#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include <queue>
#include <random>
#include <ctime>
#include <chrono>
#include <vector>
const int N = 10010;
const double INF = 2e20;

int n;
double cost[N][N];
double lx[N], ly[N], slack[N];
int prev[N];
int match[N];
bool vy[N];

void augment(int root) {
  std::fill(vy + 1, vy + n + 1, false);
  std::fill(slack + 1, slack + n + 1, INF);
  int py;
  match[py = 0] = root;
  do {
    vy[py] = true;
    int x = match[py];
    double  delta = INF, yy;
    for (int y = 1; y <= n; y++) {
      if (!vy[y]) {
        if (lx[x] + ly[y] - cost[x][y] < slack[y]) {
          slack[y] = lx[x] + ly[y] - cost[x][y];
          prev[y] = py;
        }
        if (slack[y] < delta) {
          delta = slack[y];
          yy = y;
        }
      }
    }
    for (int y = 0; y <= n; y++) {
      if (vy[y]) {
        lx[match[y]] -= delta;
        ly[y] += delta;
      } else {
        slack[y] -= delta;
      }
    }
    py = yy;
  } while (match[py] != -1);
  do {
    int pre = prev[py];
    match[py] = match[pre];
    py = pre;
  } while (py);
}

double KM(std::vector<int>& assignment) {
  assignment.resize((size_t)n, -1);
  for (int i = 1; i <= n; i++) {
    lx[i] = ly[i] = 0;
    match[i] = -1;
    for (int j = 1; j <= n; j++) {
      lx[i] = std::max(lx[i], cost[i][j]);
    }
  }
  double answer = 0;
  for (int root = 1; root <= n; root++) {
    augment(root);
  }
  for (int i = 1; i <= n; i++) {
    answer += lx[i];
    answer += ly[i];
//    printf("%d %d\n", match[i], i);
    assignment[match[i] - 1] = i - 1;
  }
  return answer;
}

double km(std::vector<int>& assignment, double c[10010][10010], int _n) {
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  int threshold = 100;
  if (n > threshold) {
    printf("Constructing KM, n = %d\n", n);
  }
  n = _n;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= n; j++) {
      cost[i][j] = c[i-1][j-1];
    }
  }
  if (n > threshold) {
    printf("Starting KM\n");
  }
  auto ret = KM(assignment);
  end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;
  if (n > threshold) {
    std::cout << "KM elapsed time: " << elapsed_seconds.count() << "s\n";
  }
  return ret;
}

