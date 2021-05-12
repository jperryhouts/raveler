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

#include <Magick++.h>

using namespace std;

string
path2latex(const vector<int> &path,
           const int row_width,
           const int k);

int
load_image(const std::string &fname,
           std::vector<double> &pixels,
           const int res,
           const bool white_thread);

void
print_help();