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
#include "ravelcli.h"
#include <sstream>

void
print_help()
  {
    std::cout << "usage: img2thread [args] <INPUT>\n\n"

              << "Raveler  Copyright (C) 2021 Jonathan Perry-Houts\n"
              << "This program comes with ABSOLUTELY NO WARRANTY.\n"
              << "This is free software, and you are welcome to redistribute it\n"
              << "under certain conditions. See the LICENSE file in the project's\n"
              << "source code repository for details:\n"
              << "https://raw.githubusercontent.com/jperryhouts/raveler/main/LICENSE\n\n"

              << "arguments:\n"
              << "  --help,-h            Show this message and quit\n"
              << "  --invert,-i          Invert the image (use black thread on white canvas)\n"
              << "  --output,-o <OUTPUT> Specify an output location (default: print to stdout)\n"
              << "  --num-pins,-k <K>    Number of pins on your frame (default: 300)\n"
              << "  --num-lines,-n <N>   Number of lines to draw before stopping (default: 6000)\n"
              << "  --weight,-w <WEIGHT> Thickness of the thread in meters (default: 100e-6)\n"
              << "  --res,-r <RES>       Before processing, scale the input image to this pixel\n"
              << "                       size along its shortest axis (default: 600)\n"
              << "  --size,-s <SIZE>     Diameter of your frame in meters (default: 0.622)\n"
              << "  --format,-f <FMT>    Output format. Can be any of csv|tsv|json|svg|tex|png|show\n"
              << "                       (default: csv)\n"
              << "  --oversample,-x <X>  Effectively increase the input resolution by oversampling\n"
              << "                       the image mask paths with factor 'X' (default: 1)\n\n"
              << "<INPUT>                Source image. Can be any image format. Use \"-\"\n"
              << "                       to read from stdin.\n"
              << endl;
  }

string
path2latex(const vector<int> &path,
           const int row_width,
           const int k)
  {
    std::stringstream result;
    result << "\\documentclass[letterpaper,twocolumn]{article}\n"
      << "\n"
      << "\\usepackage{amsfonts}\n"
      << "\\usepackage[english]{babel}\n"
      << "\\usepackage[T1]{fontenc}\n"
      << "\\usepackage[utf8]{inputenc}\n"
      << "\\usepackage{fullpage}\n"
      << "\\usepackage{array}\n"
      << "\\usepackage[table]{xcolor}\n"
      << "\\usepackage{tabularx}\n"
      << "\\usepackage{tikz}\n"
      << "\n"
      << "\\definecolor{lightgray}{gray}{0.75}\n"
      << "\n"
      << "\\newcolumntype{T}{>{\\tiny}r} % define a new column type for \\tiny\n"
      << "\n"
      << "\\begin{document}\n"
      << "\n";

    { // Generate the pin index table
      result << "\\rowcolors{1}{}{lightgray}\n";
      result << "\\begin{tabularx}{0.95\\linewidth}{T |";
      for (int i=0; i< row_width; ++i)
        result << " X";
      result << "}\n";

      int pos = 0;
      while (pos < path.size())
        {
          result << pos/row_width+1 << " & ";
          for (int j=0; j<row_width; ++j, ++pos)
            {
              if (pos < path.size())
                result << path[pos];
              else
                result << " {} ";

              if (j < row_width-1)
                result << " & ";
            }

          if (pos < path.size())
            {
              if (pos/row_width%10 == 9)
                {
                  result << "\n\\end{tabularx}\n";
                  result << "\n\\vspace{1.86mm}\n\n";
                  result << "\\rowcolors{1}{}{lightgray}\n";
                  result << "\\begin{tabularx}{0.95\\linewidth}{T |";
                  for (int i=0; i< row_width; ++i)
                    result << " X";
                  result << "}\n";
                }
              else
                {
                  result << "\\\\\n";
                }
            }
        }
      result << "\n\\end{tabularx}\n\n";
    }

    result << "\\end{document}\n\n";

    return result.str();
  }

int
load_image(const string &fname,
           vector<double> &pixels,
           const int res,
           const bool white_thread)
  {
    Magick::Image image;
    try
      {
        image.read(fname);
        image.type(Magick::GrayscaleType);

        Magick::Geometry size = image.size();
        size_t width = size.width(), height = size.height();

        if (size.width() != size.height())
          {
            size_t cx = width/2, cy = height/2;
            size_t small_side = width < height ? width : height;
            size_t xOffset = (width-small_side)/2;
            size_t yOffset = (height-small_side)/2;
            Magick::Geometry crop_region(small_side, small_side, xOffset, yOffset);
            image.crop(crop_region);
          }

        image.resize(Magick::Geometry(res,res));
        if (!white_thread)
          image.negate(true);
        // image.normalize();
        image.flip();

        image.write(0, 0, res, res, "R", Magick::DoublePixel, &pixels[0]);
      }
    catch(Magick::Exception &error_)
      {
        cerr << "Caught exception: " << error_.what() << endl;
        return 1;
      }
    return 0;
  }

int main(int argc, char* argv[])
  {
    Magick::InitializeMagick(*argv);

    int k=300, N=6000, res=600, oversample = 1;
    float weight = 100e-6, frame_size = 0.622;
    string input = "";
    string output = "-";
    string format = "csv";
    bool white_thread = false;

    int i=1;
    for (; i < argc; ++i)
      {
        string arg(argv[i]);
        if (arg == "-h" || arg == "--help")
          {
            print_help();
            return 0;
          }
        else if (arg == "-i" || arg == "--invert")
          white_thread = true;
        else if (arg == "-k" || arg == "--num-pins")
          sscanf(argv[++i], "%d", &k);
        else if (arg == "-n" || arg == "--num-lines" || arg == "-N")
          sscanf(argv[++i], "%d", &N);
        else if (arg == "-w" || arg == "--weight")
          sscanf(argv[++i], "%f", &weight);
        else if (arg == "-r" || arg == "--res")
          sscanf(argv[++i], "%d", &res);
        else if (arg == "-s" || arg == "--size")
          sscanf(argv[++i], "%f", &frame_size);
        else if (arg == "-f" || arg == "--format")
          format = argv[++i];
        else if (arg == "-o" || arg == "--output")
          output = argv[++i];
        else if (arg == "-x" || arg == "--oversample")
          sscanf(argv[++i], "%d", &oversample);
        else
          input = arg;
      }

    if (input == "")
      {
        cerr << "No source image specified.\n"
            << "Use -h flag for usage info." << endl;
        return 1;
      }

    vector<double> image;
    if (input == "-")
      {
        vector<unsigned char> raw(istreambuf_iterator<char>(std::cin),
                                  istreambuf_iterator<char>());
        res = (int) sqrt(raw.size());

        image.resize(raw.size());
        for (int i=0; i<raw.size(); ++i)
          image[i] = 1.0 - raw[i]/255.0;
      }
    else
      {
        image.resize(res*res);
        int status = load_image(input, image, res, white_thread);
        if (status != 0)
          return status;
      }

    const int max_line_length = (int) oversample * sqrt(2*res*res);
    vector<vector<int>> lines(k*k, vector<int>(max_line_length, -1));
    Raveler::fill_line_masks(k, res, oversample, lines);

    vector<double> scores(N);
    vector<int> path(N+1);
    const double relative_weight = weight * res / frame_size / oversample;
    Raveler::do_ravel(image, relative_weight, k, N, lines, path, scores);

    const double thread_length = Raveler::get_length(path, k, frame_size);

    // output data stream:
    streambuf* buf;
    ofstream of;
    if(output == "-" || format == "png" || format == "show") {
      buf = std::cout.rdbuf();
    } else {
      of.open(output, ios::out);
      buf = of.rdbuf();
    }
    std::ostream result(buf);

    if (format == "tsv" || format == "csv")
      {
        string sep = (format == "csv") ? "," : "\t";
        result << "#total thread length: " << thread_length << endl;
        result << "#pin" << sep << "score" << sep
              << "coord_x" << sep << "coord_y" << endl;
        for (int i=0; i<path.size(); ++i)
          {
            pair<double,double> xy = Raveler::pin_to_xy(path[i], k);
            result << path[i]  << sep << scores[i] << sep
                  << xy.first << sep << xy.second << endl;
          }
      }
    else if (format == "svg")
      {
        const int i_frame_size = (int) (1000 * frame_size);
        result << "<svg xmlns=\"http://www.w3.org/2000/svg\""
          << " viewbox=\"0 0 " << i_frame_size << " " << i_frame_size << "\">"
          << endl;

        result << "  <rect"
          << " width=\"" << i_frame_size << "\""
          << " height=\"" << i_frame_size << "\""
          << " fill=\""
          << (white_thread ? "black" : "white")
          << "\"/>" << endl;

        double stroke_width = weight*1000;
        string stroke_color = white_thread ? "white" : "black";
        for (int i=0; i<path.size()-1; ++i)
          {
            pair<double,double> xy0 = Raveler::pin_to_xy(path[i], k);
            pair<double,double> xy1 = Raveler::pin_to_xy(path[i+1], k);
            result << "  <line stroke=\"" << stroke_color << "\""
              << " stroke-width=\"" << stroke_width << "\""
              << " x1=\"" << i_frame_size * xy0.first  << "\""
              << " y1=\"" << i_frame_size * (1.0 - xy0.second) << "\""
              << " x2=\"" << i_frame_size * xy1.first  << "\""
              << " y2=\"" << i_frame_size * (1.0 - xy1.second) << "\" />"
              << endl;
          }

        result << "</svg>" << endl;
      }
    else if (format == "json")
      {
        result << "{" << std::endl;

        result << "  \"length\": " << thread_length << "," << endl;

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
          result << "]," << endl;
        }

        {
          result << "  \"coords\": [";
          pair<double,double> xy;
          for (int i=0; i<path.size()-1; ++i)
            {
              xy = Raveler::pin_to_xy(path[i], k);
              result << "[" << xy.first << "," << xy.second << "],";
            }
          xy = Raveler::pin_to_xy(path[path.size()-1], k);
          result << "[" << xy.first << "," << xy.second << "]";
          result << "]" << endl;
        }

        result << "}" << endl;
      }
    else if (format == "tex")
      {
        result << path2latex(path, 5, k);
      }
    else if (format == "png" || format == "show")
      {
        const int im_size = 1000;
        const Magick::Quantum bg = (white_thread ? 0 : MaxRGB);
        const Magick::Quantum fg = (white_thread ? MaxRGB : 0);

        Magick::Image im_out(
          Magick::Geometry(im_size, im_size),
          Magick::Color(bg, bg, bg, 0));

        im_out.strokeWidth(1);
        im_out.strokeAntiAlias(true);

        double pixel_diameter_meters = frame_size/im_size;
        double thread_pixel_fraction = weight/pixel_diameter_meters;
        im_out.fillColor( Magick::Color(bg, bg, bg, MaxRGB) );
        im_out.strokeColor(Magick::Color(fg, fg, fg, MaxRGB*(1.0-thread_pixel_fraction)));

        pair<double,double> xy0, xy1;
        for (int i=0; i<path.size()-1; ++i)
          {
            xy0 = Raveler::pin_to_xy(path[i], k);
            xy1 = Raveler::pin_to_xy(path[i+1], k);
            im_out.draw(Magick::DrawableLine(
              im_size*xy0.first, im_size*(1.0-xy0.second),
              im_size*xy1.first, im_size*(1.0-xy1.second)));
          }

        if (format == "png")
          {
            im_out.magick("PNG");
            im_out.write(output);
          }
        else // show
          {
            im_out.display();
          }
      }
    else
      {
        cerr << "Unknown output type: <" << format << ">" << endl;
        cerr << "  Should be one of: csv|tsv|svg|json" << endl;
      }

    if(output != "-") {
      of.close();
    }

    return 0;
  }
