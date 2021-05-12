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

// Nomenclature:
//   res: Resolution of image along one side
//        (Images are assumed to be square)
//   k:   Number of pins around circumfrence
//   i,j:  Pixel location in row/column notation
//         within 2D image matrix.
//   loc: Flattened pixel location (i*res+j)
//   x,y: Normalized location between 0 and 1.0
//

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <assert.h>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;

namespace Raveler
{
  using namespace Raveler;

  inline
  int
  ij_to_loc(const int i,
            const int j,
            const int res)
    {
      return res*j + i;
    }

  inline
  int
  xy_to_loc(const double x,
            const double y,
            const int res)
    {
      int i = (int) ((res-1) * x);
      int j = (int) ((res-1) * y);
      return ij_to_loc(i, j, res);
    }

  inline
  pair<double,double>
  pin_to_xy(const int pin,
            const int k)
    {
      double theta = 2.0 * M_PI * pin / k;
      double x = 0.5 + sin(theta)/2.0;
      double y = 0.5 + cos(theta)/2.0;
      return pair<double,double>(x, y);
    }

  inline
  int
  pin_to_loc (const int pin,
              const int k,
              const int res)
    {
      pair<double,double> pos = pin_to_xy(pin, k);
      return xy_to_loc(pos.first, pos.second, res);
    }

  /*
  * Find locations of pixels along line between two points.
  *
  * Arguments:
  *   loc0, loc1: Pixel locations in flattened indices
  *   res: Size of image (total pixels = res^2)
  *   line_buffer*: Vectors to be filled with pixel indices
  *
  * Returns:
  *   Length of line, measured in pixels.
  */
  int
  get_line(const int loc0,
          const int loc1,
          const int res,
          const int oversample,
          vector<int> &line_buffer1,
          vector<int> &line_buffer2);

  /* Pre-compute pixel masks for each thread pair; i.e.
  * A line from pin A to B would pass through certain
  * pixels in a (res x res) size image.
  *
  * This function returns a 2D vector with k^2 rows
  * representing pairs of pins (indexed as a*k+b).
  * Values within each row describe a sequence of pixels
  * that are included in the mask (indexed as i*res+j).
  * All rows have constant length, set to the maximum
  * length of any pixel mask.
  * The vector should be filled with -1's on itialization.
  *
  * Arguments:
  *   k: Total number of pins
  *   res: Width of the image in pixels (e.g. 100 for
  *        an image with size 100x100).
  *   lines: Pixel masks for each pin-pin combination, with
  *          layout and indexing as described above.
  */
  void
  fill_line_masks(const int k,
                  const int res,
                  const int oversample,
                  vector<vector<int>> &lines);

  double
  get_score(const int a,
            const int b,
            const int k,
            const double weight,
            const vector<double> &residual,
            const vector<vector<int>> &lines);

  void
  do_ravel( const vector<double> &img,
            const double weight,
            const int k,
            const int N,
            const vector<vector<int>> &lines,
            vector<int> &path,
            vector<double> &scores);

  double
  get_length(const vector<int> &path,
            const int k,
            const double frame_size);
}