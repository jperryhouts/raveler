
#include <iostream>
#include <vector>

#include <Magick++.h>

using namespace std;

int
load_image(const string &fname,
           vector<double> &pixels,
           const int res,
           const bool white_thread);
