#include "img2thread.h"

void
print_help()
  {
    std::cout << "usage: img2thread [args] <INPUT>\n\n"
              << "arguments:\n"
              << "  --help,-h      Show this message and quit\n"
              << "  --output,-o    Specify an output location (use \"-\" for stdin)\n"
              << "  --num-pins,-k  Number of pins on your frame (default: 300)\n"
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

int
load_image(const string &fname,
           vector<double> &pixels,
           const int res)
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
        image.negate(true);
        image.normalize();

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

    int i=1;
    for (; i < argc; ++i)
      {
        string arg(argv[i]);
        if (arg == "-h" || arg == "--help")
          {
            print_help();
            return 0;
          }
        if (arg == "-k" || arg == "--num-pins")
          sscanf(argv[++i], "%d", &k);
        else if (arg == "-N" || arg == "--num-lines")
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
        int status = load_image(input, image, res);
        if (status != 0)
          return status;
      }

    vector<int> lengths(k*k, -1);
    vector<vector<int>> lines = get_all_lines(k, res, lengths);

    vector<double> scores(1, 0.0);
    vector<int> path(1, 1);

    // ~210 ms for 13000 lines at weight = 0.005
    stringify(image, k, lines, lengths, N, weight, path, scores);

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
        //int frame = 585; // in millimeters

        const int i_frame_size = (int) (1000 * frame_size);
        result << "<svg xmlns=\"http://www.w3.org/2000/svg\""
          << " viewbox=\"0 0 " << i_frame_size << " " << i_frame_size << "\">"
          << endl;

        double stroke_width = weight / frame_size;
        for (int i=0; i<path.size()-1; ++i)
          {
            pair<double,double> xy0 = pin_to_xy(path[i], k);
            pair<double,double> xy1 = pin_to_xy(path[i+1], k);
            result << "  <line stroke=\"black\" stroke-width=\"" << stroke_width << "\""
              << " x1=\"" << i_frame_size * xy0.first  << "\""
              << " y1=\"" << i_frame_size * xy0.second << "\""
              << " x2=\"" << i_frame_size * xy1.first  << "\""
              << " y2=\"" << i_frame_size * xy1.second << "\" />"
              << endl;
          }

        result << "</svg>" << endl;
      }
    else if (format == "json")
      {
        result << "{" << std::endl;

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
          xy = pin_to_xy(path[i], k);
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
