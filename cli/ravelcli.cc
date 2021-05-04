/* Compiling:

  c++ -o raveler ravelcli.cc `Magick++-config --cxxflags --cppflags --ldflags --libs`

*/


/* Example usage:

    convert original.jpg -gravity center -extent 1:1 -resize @10000 -size 100x100 -depth 8 GRAY:- \
        | raveler -r 100 -k 256 -N 1000 - > threads.txt

  or:

    raveler -r 100 -k 256 -w 0.05 -N 2000 original.jpg > threads.txt

  python3 -c "from pylab import *; p,s,x,y = loadtxt('threads.txt').T; plot(x,1-y,'k-',lw=110/len(p)); gca().set_aspect(1); show()"

*/

#include "libraveler.h"
#include "ravelcli.h"

void
print_help()
  {
    std::cout << "usage: img2thread [args] <INPUT>\n\n"
              << "arguments:\n"
              << "  --help,-h      Show this message and quit\n"
              << "  --invert,-i    Invert the image (use black thread on white canvas)\n"
              << "  --output,-o    Specify an output location (use \"-\" for stdin)\n"
              << "  --num-pins,-k  Number of pins on your frame (default: 300)\n"
              << "  --num-lines,-n Number of lines to draw before stopping (default: 4000)\n"
              << "  --weight,-w    Thickness of the thread in meters (default: 100e-6)\n"
              << "  --res,-r       Before processing, scale the input image to this pixel\n"
              << "                 size along its shortest axis (default: 600)\n"
              << "  --size,-s      Diameter of your frame in meters (default: 0.622)\n"
              << "  --format,-f    Output format. Can be any of csv|tsv|json|svg\n"
              << "                 (default: csv)\n\n"
              << "INPUT            Source image. Can be any image format. Use \"-\"\n"
              << "                 to read from stdin.\n"
              << endl;
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

    // originals: w = 0.072mm / frame = 0.622m

    int k=300, N=4000, res=600;
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
        else
          input = arg;
      }

    if (input == "")
      {
        cerr << "No source image specified.\n"
            << "Use -h flag for usage info." << endl;
        return 1;
      }

    weight = (weight == 0) ? 50.0/N : weight;
    weight *= 1000;

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

    vector<int> lengths(k*k, -1);
    vector<vector<int>> lines = get_all_lines(k, res, lengths);

    vector<double> scores(1, 0.0);
    vector<int> path(1, 0);

    // ~210 ms for 13000 lines at weight = 0.005
    stringify(image, k, lines, lengths, N, weight, path, scores);

    const double thread_length = get_length(path, k, frame_size);

    // output data stream:
    streambuf* buf;
    ofstream of;
    if(output == "-") {
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
            pair<double,double> xy = pin_to_xy(path[i], k);
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

        double stroke_width = weight / frame_size;
        string stroke_color = white_thread ? "white" : "black";
        for (int i=0; i<path.size()-1; ++i)
          {
            pair<double,double> xy0 = pin_to_xy(path[i], k);
            pair<double,double> xy1 = pin_to_xy(path[i+1], k);
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
              xy = pin_to_xy(path[i], k);
              result << "[" << xy.first << "," << xy.second << "],";
            }
          xy = pin_to_xy(path[path.size()-1], k);
          result << "[" << xy.first << "," << xy.second << "]";
          result << "]" << endl;
        }

        result << "}" << endl;

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
