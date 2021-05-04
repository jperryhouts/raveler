
#include <iostream>
#include <vector>

#include <Magick++.h>

using namespace std;

int
load_image(const std::string &fname,
           std::vector<double> &pixels,
           const int res,
           const bool white_thread);

void
print_help();