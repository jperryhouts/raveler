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

// #include <iostream>
#include <sstream>
#include <cstring>

#include "libraveler.h"
#include "raveljs.h"

string
design_to_json(const design d)
  {
    std::stringstream result;

    result << "{" << endl;
    result << "  \"length\": " << d.length << "," << endl;

    {
      result << "  \"pins\": [";
      for (int i=0; i<d.path.size()-1; ++i)
        result << d.path[i] << ",";
      result << d.path[d.path.size()-1];
      result << "]," << endl;
    }

    {
      result << "  \"scores\": [";
      for (int i=0; i<d.scores.size()-1; ++i)
        result << d.scores[i] << ",";
      result << d.scores[d.scores.size()-1];
      result << "]" << endl;
    }

    result << "}" << endl;

    return result.str();
  }

extern "C"
{
  char*
  ravel(float weight, float frame_size)
    {
      cout << "[ravel] ready" << endl;

      vector<double> image(RES2);
      for (int idx=0; idx<RES2; idx++)
        {
          double px = (double) global.pixel_buffer[idx];
          image[idx] = 1.0 - px / 255.0;
        }

      cout << "[ravel] set" << endl;

      design d;
      d.path.resize(N+1);
      d.scores.resize(N);
      Raveler::do_ravel(image, 1e3*weight/frame_size,
                        K, N, global.line_masks,
                        d.path, d.scores);
      d.length = Raveler::get_length(d.path, K, frame_size);

      const string result = design_to_json(d);
      char* ret = new char[result.length()+1];
      std::strcpy(ret, result.c_str());

      cout << "[ravel] done" << endl;
      return ret;
    }

  int
  init()
    {
      global.line_masks.resize(K2, vector<int>(850, -1));
      Raveler::fill_line_masks(K, RES, global.line_masks);
      return (int) global.pixel_buffer;
    }
}
