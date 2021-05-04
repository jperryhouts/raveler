//
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
 *
 * All rows have constant length, set to the maximum
 * length of any pixel mask. The 'lengths' argument
 * should be a k^2 length vector, and will be filled
 * with values describing the actual length of each
 * pixel mask, using the same index notation as above.
 *
 * Arguments:
 *   k: Total number of pins
 *   res: Width of the image in pixels (e.g. 100 for
 *        an image with size 100x100).
 *   lengths: Pointer to an integer vector with length
 *            k^2. Will be filled with the lengths of
 *            each pixel mask line.
 *
 * Returns:
 *   Pixel masks for each pin-pin combination, with
 *   layout and indexing as described above.
 */
vector<vector<int>>
get_all_lines(const int k,
              const int res,
              vector<int> &lengths);

double
get_score(const int a,
          const int b,
          const int k,
          const double weight,
          const vector<double> &residual,
          const vector<vector<int>> &lines,
          const vector<int> &line_lengths);

void
stringify(const vector<double> &img,
          const int k,
          const vector<vector<int>> &lines,
          const vector<int> &line_lengths,
          const int N,
          const double weight,
          vector<int> &path,
          vector<double> &scores);

double
get_length(const vector<int> &path,
           const int k,
           const double frame_size);
