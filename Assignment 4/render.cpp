// Skeleton code for B657 A4 Part 1.
// D. Crandall
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>

using namespace cimg_library;
using namespace std;

CImg<double> part_1(CImg<double> &image_rgb, CImg<double> &image_disp)
{

  CImg<double> img_temp(image_rgb.width(), image_rgb.height(),1,3,0);
  CImg<double> image_result(image_rgb.width(), image_rgb.height(),1,3,0);
  double constant = 0.06;

  for(int i = 0; i<image_rgb.width(); i++)
  {
    for(int j = 0; j<image_rgb.height(); j++)
    {
      double t =  constant * image_disp(i,j);
      if(i - t >=0 && i-t < image_rgb.width())
      {
        img_temp(i,j,0,0) = image_rgb((i- t),j,0,0);
        img_temp(i,j,0,1) = image_rgb((i-t),j,0,1);
        img_temp(i,j,0,2) = image_rgb((i-t),j,0,2);
      }
      
    }
  }

  for(int i = 0; i<image_rgb.width(); i++)
  {
    for(int j = 0; j<image_rgb.height(); j++)
    {
      image_result(i,j,0) = img_temp(i,j,0,0);
      image_result(i,j,1) = image_rgb(i,j,0,1);
      image_result(i,j,2) = image_rgb(i,j,0,2);

    }
  }

  return image_result;

}

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
      cerr << "usage: " << argv[0] << " image_file disp_file" << endl;
      return 1;
    }

  string input_filename1 = argv[1], input_filename2 = argv[2];

  // read in images and gt
  CImg<double> image_rgb(input_filename1.c_str());
  CImg<double> image_disp(input_filename2.c_str());

  cout << image_rgb.width() * image_rgb.height() << endl;

  CImg<double> image_result = part_1(image_rgb, image_disp);

  image_result.get_normalize(0,255).save((input_filename1 + "-stereogram.png").c_str());

  return 0;
}