#include "img2thread.h"

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

    int k=256, N=1000, res=100;
    float weight = 0;
    string fname;

    int i=1;
    for (; i < argc; ++i)
      {
        string arg(argv[i]);
        if (arg == "-k")
          sscanf(argv[++i], "%d", &k);
        else if (arg == "-N")
          sscanf(argv[++i], "%d", &N);
        else if (arg == "-w")
          sscanf(argv[++i], "%f", &weight);
        else if (arg == "-r")
          sscanf(argv[++i], "%d", &res);
        else
          fname = arg;
      }

    weight = (weight == 0) ? 50.0/N : weight;

    vector<double> image;
    if (fname == "-")
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
        int status = load_image(fname, image, res);
        if (status != 0)
          return status;
      }

    vector<int> lengths(k*k, -1);
    vector<vector<int>> lines = get_all_lines(k, res, lengths);

    vector<double> scores(1, 0.0);
    vector<int> path(1, 1);

    // ~210 ms for 13000 lines at weight = 0.005
    stringify(image, k, lines, lengths, N, weight, path, scores);

    for (int i=0; i<path.size(); ++i)
      {
        pair<double,double> xy = pin_to_xy(path[i], k);
        cout << path[i] << " " << scores[i] << " "
            << xy.first << " " << xy.second << endl;
      }

    return 0;
  }