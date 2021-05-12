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

#include "libraveler.h"

namespace Raveler
{
  using namespace Raveler;

  int
  get_line(const int loc0,
          const int loc1,
          const int res,
          const int oversample,
          vector<int> &line_buffer1,
          vector<int> &line_buffer2)
    {
      int cx0 = loc0%res, cy0 = loc0/res;
      int cx1 = loc1%res, cy1 = loc1/res;
      double dx = cx1 - cx0;
      double dy = cy1 - cy0;
      int R = (int) (oversample * sqrt(dx*dx + dy*dy));

      for (int i=0; i<R; ++i)
        {
          double denominator = (R == oversample) ? 1.0 : (R/oversample-1.0);
          int p_i = cx0 + i/oversample*dx/denominator;
          int p_j = cy0 + i/oversample*dy/denominator;
          int loc = ij_to_loc(p_i, p_j, res);
          line_buffer1[i] = loc;
          line_buffer2[i] = loc;
        }
      return R;
    }

  void
  fill_line_masks(const int k,
                  const int res,
                  const int oversample,
                  vector<vector<int>> &lines)
  {
      vector<int> pin_locs(k);
      for (int pos=0; pos<k; ++pos)
        pin_locs[pos] = pin_to_loc(pos, k, res);

      for (int i=0; i<k; ++i)
        for (int j=i; j<k; ++j)
          {
            int l1 = i*k + j;
            int l2 = j*k + i;
            get_line(pin_locs[i], pin_locs[j], res, oversample,
                     lines[l1], lines[l2]);
          }
  }

  double
  get_score(const int a,
            const int b,
            const int k,
            const double weight,
            const vector<double> &residual,
            const vector<vector<int>> &lines)
    {
      const int idx = a*k + b;
      double integrated_residual = 0.0;
      int pos=0;
      for (; lines[idx][pos] != -1; pos++)
        integrated_residual += residual[lines[idx][pos]];
      double score = weight * (2 * integrated_residual - weight * pos);
      return score;
    }

  void
  do_ravel( const vector<double> &image,
            const double weight,
            const int k,
            const int N,
            const vector<vector<int>> &lines,
            vector<int> &path,
            vector<double> &scores)
    {
      // For whatever reason, the visual effect of a strand of
      // thread crossing any particular region seems to be lower
      // than what you'd predict. This 0.7 scale factor seems
      // to produce output that looks roughly true to reality.
      const double visual_weight = 0.7 * weight;

      vector<double> residual(image);

      path[0] = 0;
      int path_size = 1;
      for (int path_size=1; path_size <= N; path_size++)
        {
          int previous_pin = path[path_size-1];
          int next_pin;
          double score = -1e20;

          for (int pin=0; pin<k; ++pin)
            {
              bool recently_visited = false;

              for (int n=1; (n < 3) && (n < path_size); ++n)
                recently_visited |= (path[path_size-n] == pin);

              if (recently_visited)
                continue;

              double pin_score = get_score(previous_pin, pin, k, visual_weight,
                                           residual, lines);
              if (pin_score > score)
                {
                  score = pin_score;
                  next_pin = pin;
                }
            }

          path[path_size] = next_pin;
          scores[path_size-1] = score;

          int line_idx = previous_pin*k + next_pin;
          for (int i=0; lines[line_idx][i] != -1; ++i)
            residual[lines[line_idx][i]] -= visual_weight;
        }
    }

  double
  get_length(const vector<int> &path,
            const int k,
            const double frame_size)
    {
      double length = 0;
      for (int i=0; i<path.size()-1; ++i)
        {
          pair<double,double> xy0 = pin_to_xy(path[i], k);
          pair<double,double> xy1 = pin_to_xy(path[i+1], k);
          double dx = frame_size * (xy1.first - xy0.first);
          double dy = frame_size * (xy1.second - xy0.second);
          length += sqrt(dx*dx + dy*dy);
        }
      return length;
    }
}