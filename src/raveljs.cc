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

#include <sstream>
#include <cstring>

#include "libraveler.h"
#include "raveljs.h"

string
do_ravel(float weight, float frame_size)
  {
    vector<int> path(N+1);
    vector<double> scores(N);

    Raveler::do_ravel(global_image, 1e3*weight/frame_size, K, N, global_line_masks, path, scores);

    std::stringstream result;

    result << "{" << endl;
    result << "  \"length\": " << Raveler::get_length(path, K, frame_size) << "," << endl;

    {
      result << "  \"pins\": [";
      for (int i=0; i<path.size()-1; ++i)
        result << path[i] << ",";
      result << path[path.size()-1];
      result << "]," << endl;
    }

    {
      result << "  \"scores\": [";
      for (int i=0; i<scores.size()-1; ++i)
        result << scores[i] << ",";
      result << scores[scores.size()-1];
      result << "]" << endl;
    }

    result << "}" << endl;

    return result.str();
  }

extern "C"
{
  /*
  init = Module.cwrap('init', null, []); init();
  arr = new Uint8Array(600*600);
  for (let i=0; i<600*600; i++) arr[i] = (Math.random()*255).toFixed();
  img_buffer = Module._malloc(arr.length*arr.BYTES_PER_ELEMENT);
  Module.HEAPU8.set(arr, img_buffer);
  ravel = Module.cwrap('ravel', 'string', ['number','number','number']);
  result = JSON.parse(ravel(100e-6, 0.622, img_buffer));
  free = Module.cwrap('free_mem', null, ["number"], [img_buffer]); free(img_buffer);
  console.log(result);
  */

  void
  free_mem(int* loc)
    {
      free(loc);
    }

  char*
  ravel(float weight, float frame_size, unsigned char* pixels)
    {
      cout << "ready" << endl;

      for (int idx=0; idx<RES2; idx++)
        {
          double px = (double) pixels[idx];
          global_image[idx] = 1.0 - px / 255.0;
        }

      cout << "set" << endl;

      const string result = do_ravel(weight, frame_size);

      cout << "finished" << endl;

      char* ret = new char[result.length()+1];
      std::strcpy(ret, result.c_str());
      return ret;
    }

  void
  init()
    {
      Raveler::fill_line_masks(K, RES, global_line_masks);
    }
}
