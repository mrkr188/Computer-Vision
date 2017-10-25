// Skeleton code for B657 A4 Part 3.
// D. Crandall
//
// Run like this, for example:
//   ./stereo part3/Aloe/view1.png part3/Aloe/view5.png part3/Aloe/gt.png
// and output files will appear in part3/Aloe
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>
#include <limits>

#define LABELS 50
#define ITR 30
#define BETA 5
#define WINDOW 5
#define ALPHA 250

using namespace cimg_library;
using namespace std;



//Created By: Marshal Patel
class Pixel
{
public:
  
  
  double msg[LABELS+1];
  //vector<Pixel> nghbrs;
  //Pixel nghbrs[4];

  Pixel()
  {
    
    //msg[0] = 0;
   // msg[1] = 0;
    for(int i = 0; i<LABELS+1; i++)
    {
      msg[i] = 0;
    }
   
  }

};

double sqr(double a) { return a*a; }

// This code may or may not be helpful. :) It computes a 
//  disparity map by looking for best correspondences for each
//  window independently (no MRF).
//
CImg<double> naive_stereo(const CImg<double> &input1, const CImg<double> &input2, int window_size, int max_disp)
{  
  CImg<double> result(input1.width(), input1.height());

  for(int i=0; i<input1.height(); i++)
    for(int j=0; j<input1.width(); j++)
      {
	pair<int, double> best_disp(0, INFINITY);

	for (int d=0; d < max_disp; d++)
	  {
	    double cost = 0;
	    for(int ii = max(i-window_size, 0); ii <= min(i+window_size, input1.height()-1); ii++)
	      for(int jj = max(j-window_size, 0); jj <= min(j+window_size, input1.width()-1); jj++)
		cost += sqr(input1(min(jj+d, input1.width()-1), ii) - input2(jj, ii));

	    if(cost < best_disp.second)
	      best_disp = make_pair(d, cost);
	  }
	result(j,i) = best_disp.first;
      }

  return result;
}



double smootheningFn(int l1, int l2)
{
  if(l1==l2)
    return 0;
  else
    return 1;
}



double computeDataCost(int x, int y, int d, const CImg<double> &left, const CImg<double> &right)
{

  double cost = 0;

  for(int u = -1 * WINDOW; u<=WINDOW; u++)
  {
    for(int v = -1 * WINDOW; v<=WINDOW; v++)
    {
        if(y+v < left.width() && y+v+d < left.width() && x+u < left.height() && y+v >= 0 && y+v+d >= 0 && x+u >= 0)
        cost += (left(y+v,x+u) - right(y+v+d,x+u)) * (left(y+v,x+u) - right(y+v+d,x+u));
    }
  }

  return cost;

  
}



double getMessageFromTop(int x, int y, CImg<Pixel> &message_map, int label)
  {
    return message_map(y,x-1).msg[label];
  }



  double getMessageFromRight(int x, int y, CImg<Pixel> &message_map, int label)
  {
    return message_map(y+1,x).msg[label];
  }



  double getMessageFromBottom(int x, int y, CImg<Pixel> &message_map, int label)
  {
    return message_map(y,x+1).msg[label];
  }



  double getMessageFromLeft(int x, int y, CImg<Pixel> &message_map, int label)
  {
    return message_map(y-1,x).msg[label];
  }




double send(int x, int y, int dir, CImg<Pixel> &message_map, const CImg<double> &left, const CImg<double> &right, int l)
  {
    
    double min_cost = numeric_limits<double>::max();

    for(int label = 0; label<=LABELS; label++)
    {
      double cost = 0;
      cost += computeDataCost(x, y, label, left, right);

        cost += smootheningFn(l, label) * ALPHA;

      switch(dir)
      {
        case 0:
          cost += getMessageFromLeft(x, y, message_map, label);
          cost += getMessageFromRight(x, y, message_map, label);
          cost += getMessageFromBottom(x, y, message_map, label);
          break;

          case 1:
          cost += getMessageFromLeft(x, y, message_map, label);
          cost += getMessageFromTop(x, y, message_map, label);
          cost += getMessageFromBottom(x, y, message_map, label);
          break;

          case 2:
          cost += getMessageFromLeft(x, y, message_map, label);
          cost += getMessageFromTop(x, y, message_map, label);
          cost += getMessageFromRight(x, y, message_map, label);
          break;

          case 3:
          cost += getMessageFromBottom(x, y, message_map, label);
          cost += getMessageFromTop(x, y, message_map, label);
          cost += getMessageFromRight(x, y, message_map, label);
          break;
      }

      
      

      if(min_cost > cost)
        min_cost = cost;

    }

    return min_cost;
  }




int belief(int x, int y, CImg<Pixel> &message_map, const CImg<double> &left, const CImg<double> &right)
{


  double min_cost = numeric_limits<double>::max();
  int l = 0;

    for(int label = 0; label<=LABELS; label++)
    {
      double cost = 0;
      cost += computeDataCost(x, y, label, left, right);


      if(cost < numeric_limits<double>::infinity())
      {

        
          cost += getMessageFromTop(x,y,message_map,label);
          cost += getMessageFromLeft(x, y, message_map, label);
          cost += getMessageFromRight(x, y, message_map, label);
          cost += getMessageFromBottom(x, y, message_map, label);
        
      }
      

      if(min_cost > cost)
      {
        min_cost = cost;
        l = label;
      }
        

    }

    return l;

}


CImg<double> mrf_stereo(const CImg<double> &img1, const CImg<double> &img2)
{
  /*CImg<double> result(img1.width(), img1.height());

  for(int i=0; i<img1.height(); i++)
    for(int j=0; j<img1.width(); j++)
      result(j,i) = rand() % 256;*/

    CImg<Pixel> message_map(img1.width(), img1.height(),1,1);
    CImg<Pixel> p_message_map(img1.width(), img1.height(),1,1);

    CImg<double> result(img1.width(), img1.height(),1,1,0);
    
    for(int itr = 0; itr<ITR; itr++)
    {

      cout << "Iteration: " << itr << endl;

      for(int i = 1; i<img1.width()-1; i++)
    {
      for(int j = 1; j<img1.height()-1; j++)
      {
        for(int label = 0; label <= LABELS; label++)
        {
          for(int dir = 0; dir<4; dir++)
          {
            
            if(dir == 0 && j-1>=0)
            {
                message_map(i,j-1).msg[label] = send(j,i, dir,p_message_map, img1, img2, label);
            }

            if(dir == 1 && i + 1 < img1.width())
            {
                message_map(i+1,j).msg[label] = send(j,i, dir,p_message_map, img1, img2, label);
            }

            if(dir == 2 && j+1 < img1.height())
            {
                message_map(i,j+1).msg[label] = send(j,i, dir,p_message_map, img1, img2, label);
            }

            if(dir == 3 && i-1>=0)
            {
                message_map(i-1,j).msg[label] = send(j,i, dir,p_message_map, img1, img2, label);
            }


          }
        }
      }

      
    }

      p_message_map = message_map;

    }

    //message_map.save_png("message_map.png");

    //double belief(int x, int y, CImg<Pixel> &message_map, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1)

    for(int i = 1; i<img1.width()-1; i++)
    {
      for(int j = 1; j<img1.height()-1; j++)
      {

          result(i,j) = belief(j,i,message_map,img1,img2);
        
      }
    }
    

    //result *= 255.0;
    //result.save_png("lbp.png");

    return result;


  
}



int main(int argc, char *argv[])
{
  if(argc != 4 && argc != 3)
    {
      cerr << "usage: " << argv[0] << " image_file1 image_file2 [gt_file]" << endl;
      return 1;
    }

  string input_filename1 = argv[1], input_filename2 = argv[2];
  string gt_filename;
  if(argc == 4)
    gt_filename = argv[3];

  // read in images and gt
  CImg<double> image1(input_filename1.c_str());
  CImg<double> image2(input_filename2.c_str());
  CImg<double> gt;

  if(gt_filename != "")
  {
    gt = CImg<double>(gt_filename.c_str());

    // gt maps are scaled by a factor of 3, undo this...
    for(int i=0; i<gt.height(); i++)
      for(int j=0; j<gt.width(); j++)
        gt(j,i) = gt(j,i) / 3.0;
  }
  
  // do naive stereo (matching only, no MRF)
  CImg<double> naive_disp = naive_stereo(image1, image2, 5, 50);
  naive_disp.get_normalize(0,255).save((input_filename1 + "-disp_naive.png").c_str());

  // do stereo using mrf
  CImg<double> mrf_disp = mrf_stereo(image1, image2);
  mrf_disp.get_normalize(0,255).save((input_filename1 + "-disp_mrf.png").c_str());

  // Measure error with respect to ground truth, if we have it...
  if(gt_filename != "")
    {
      cout << "Naive stereo technique mean error = " << (naive_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;
      cout << "MRF stereo technique mean error = " << (mrf_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;

    }


  return 0;
}
