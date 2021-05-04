#include "raveler.h"

int
get_line(const int loc0,
         const int loc1,
         const int res,
         vector<int> &line_buffer1,
         vector<int> &line_buffer2)
  {
    int cx0 = loc0%res, cy0 = loc0/res;
    int cx1 = loc1%res, cy1 = loc1/res;
    double dx = cx1 - cx0;
    double dy = cy1 - cy0;
    int R = (int) sqrt(dx*dx + dy*dy);

    for (int i=0; i<R; ++i)
      {
        double denominator = (R == 1) ? 1.0 : (R-1.0);
        int p_i = cx0 + i*dx/denominator;
        int p_j = cy0 + i*dy/denominator;
        int loc = ij_to_loc(p_i, p_j, res);
        line_buffer1[i] = loc;
        line_buffer2[i] = loc;
      }
    return R;
  }

vector<vector<int>>
get_all_lines(const int k,
              const int res,
              vector<int> &lengths)
  {
    vector<int> pin_locs(k);
    for (int pos=0; pos<k; ++pos)
      pin_locs[pos] = pin_to_loc(pos, k, res);

    const int max_line_length = (int) sqrt(2*res*res);

    vector<vector<int>> lines(k*k, vector<int>(max_line_length));
    for (int i=0; i<k; ++i)
      for (int j=i; j<k; ++j)
        {
          int l1 = i*k + j;
          int l2 = j*k + i;
          int R = get_line(pin_locs[i], pin_locs[j], res,
                          lines[l1], lines[l2]);
          lengths[l1] = R;
          lengths[l2] = R;
        }
    return lines;
  }

double
get_score(const int a,
          const int b,
          const int k,
          const double weight,
          const vector<double> &residual,
          const vector<vector<int>> &lines,
          const vector<int> &line_lengths)
  {
    const int idx = a*k + b;
    double integrated_residual = 0.0;
    for (int j=0; j<line_lengths[idx]; ++j)
      integrated_residual += residual[lines[idx][j]];
    double score = weight * (2*integrated_residual - weight * line_lengths[idx]);
    return score;
  }

void
stringify(const vector<double> &image,
          const int k,
          const vector<vector<int>> &lines,
          const vector<int> &line_lengths,
          const int N,
          const double weight,
          vector<int> &path,
          vector<double> &scores)
  {
    vector<double> residual(image);

    while (path.size() <= N)
      {
        int previous_pin = path[path.size()-1];
        int next_pin;
        double score = -1e20;

        for (int pin=0; pin<k; ++pin)
          {
            bool recently_visited = false;

            for (int n=1; (n < 3) && (n < path.size()); ++n)
              recently_visited |= (path[path.size()-n] == pin);

            if (recently_visited)
              continue;

            double pin_score = get_score(previous_pin, pin, k, weight,
                                         residual, lines, line_lengths);
            if (pin_score > score)
              {
                score = pin_score;
                next_pin = pin;
              }
          }

        path.push_back(next_pin);
        scores.push_back(score);

        int line_idx = previous_pin*k + next_pin;
        for (int i=0; i<line_lengths[line_idx]; ++i)
          residual[lines[line_idx][i]] -= weight;
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
