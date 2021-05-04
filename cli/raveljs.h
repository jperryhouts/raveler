
/*
  Raveler
  Copyright (C) 2021 Jonathan Perry-Houts

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <vector>

#define N 6000
#define K 300
#define K2 90000
#define RES 600
#define RES2 360000

using namespace std;

// Must allocate >= sqrt(2*RES2) for each line mask.
// About 850 in our case.
vector<vector<int>> global_line_masks(K2, vector<int>(850, -1));
vector<double> global_image(RES2);

string do_ravel(float weight, float frame_size);

extern "C"
{
  void init();
  char* ravel(float weight, float frame_size, unsigned char* pixels);
}
