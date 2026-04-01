#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cpp_template.hpp"

extern "C" {
#define ANSI_DECLARATORS 1
#define VOID void
#ifdef SINGLE
#define REAL float
#else
#define REAL double
#endif
#include "shewchuk/triangle.h"
}

void dt(const std::vector<std::vector<REAL>>& points) {
  std::vector<std::vector<REAL>> sorted_points = points;
  int n = static_cast<int>(sorted_points.size());
    int half = n / 2;

  std::sort(sorted_points.begin(), sorted_points.end(), [](const std::vector<REAL>& a, const std::vector<REAL>& b) {
        return a[0] < b[0]; // Sort by x-coordinate
    });

  std::vector<std::vector<REAL>> L_points(sorted_points.begin(), sorted_points.begin() + half);
  std::vector<std::vector<REAL>> R_points(sorted_points.begin() + half, sorted_points.end());
}

int main(int argc, char **argv) {
  std::string node_path = "tests/spiral.node";
  if (argc > 1) {
    node_path = argv[1];
  }

  std::ifstream node_file(node_path);
  if (!node_file) {
    std::cerr << "Error opening path: " << node_path << "\n";
    return 1;
  }

  int n = 0;
  int trash; 
  if (!(node_file >> n >> trash >> trash >> trash)) {
    std::cerr << "Error \n";
    return 1;
  }

  std::vector<std::vector<REAL>> points;
  points.reserve(n);

  for (int i = 0; i < n; i++) {
    int index;
    REAL x, y; 

    if (!(node_file >> index >> x >> y)) {
      std::cerr << "Error \n";
      return 1;
    }

    points.push_back({x, y});
  }
  print(points);

  return 0;
}
