#include "libraveler.h"
#include "raveljs.h"

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

int
main (int argc, char** argv)
{
    Magick::InitializeMagick(*argv);

    int k=300, N=4000, res=600;
    float weight = 100e-6, frame_size = 0.622;
    weight *= 1000;
    string input = "volcano3.jpg";

    vector<double> image(res*res);
    int status = load_image(input, image, res, false);
    if (status != 0)
        return status;

    vector<int> lengths(k*k, -1);
    vector<vector<int>> lines = get_all_lines(k, res, lengths);

    vector<double> scores(1, 0.0);
    vector<int> path(1, 0);

    stringify(image, k, lines, lengths, N, weight, path, scores);

    const double thread_length = get_length(path, k, frame_size);

    std::cout << "{" << std::endl;

    std::cout << "  \"length\": " << thread_length << "," << endl;

    {
        std::cout << "  \"pins\": [";
        for (int i=0; i<path.size()-1; ++i)
            std::cout << path[i] << ",";
        std::cout << path[path.size()-1];
        std::cout << "]," << endl;
    }

    {
        std::cout << "  \"scores\": [";
        for (int i=0; i<scores.size()-1; ++i)
            std::cout << scores[i] << ",";
        std::cout << scores[scores.size()-1];
        std::cout << "]," << endl;
    }

    {
        std::cout << "  \"coords\": [";
        pair<double,double> xy;
        for (int i=0; i<path.size()-1; ++i)
        {
            xy = pin_to_xy(path[i], k);
            std::cout << "[" << xy.first << "," << xy.second << "],";
        }
        xy = pin_to_xy(path[path.size()-1], k);
        std::cout << "[" << xy.first << "," << xy.second << "]";
        std::cout << "]" << endl;
    }

    std::cout << "}" << endl;

    return 0;
}
