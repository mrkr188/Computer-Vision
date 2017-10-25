// Skeleton code for B657 A4 Part 2.
// D. Crandall
//
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>
#include <limits>

#define TOP 0
#define RIGHT 1
#define BOTTOM 2
#define LEFT 3
#define BETA 5
#define LABELS 1
#define ITR 10
#define ALPHA 150

using namespace cimg_library;
using namespace std;



class Point
{
public:
  Point() {}
  Point(int _col, int _row) : row(_row), col(_col) {}
  int row, col;
};


class Pixel
{
public:
  
  Point pos;
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


CImg<double> naive_segment(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1)
{
  // implement this in step 2...
  //  this placeholder just returns a random disparity map
  CImg<double> result(img.width(), img.height());
  //gpd(img.width(), img.height());
  
  double mean1, mean2, mean3;
  double variance1, variance2, variance3; 
  
  double fg_size = fg.size();
  double bg_size = bg.size();
  
  for(int f=0; f < fg_size; f++){
    mean1 = mean1 + img(fg[f].col, fg[f].row, 0, 0);
    mean2 = mean2 + img(fg[f].col, fg[f].row, 0, 1);
    mean3 = mean3 + img(fg[f].col, fg[f].row, 0, 2);
  }
  
  mean1 = mean1/fg_size;
  mean2 = mean2/fg_size;
  mean3 = mean3/fg_size;

  for(int f=0; f < fg_size; f++){
    variance1 = variance1 += (img(fg[f].col, fg[f].row, 0, 0) - mean1) * (img(fg[f].col, fg[f].row, 0, 0) - mean1) ;
    variance2 = variance2 += (img(fg[f].col, fg[f].row, 0, 1) - mean2) * (img(fg[f].col, fg[f].row, 0, 1) - mean2) ;
    variance3 = variance3 += (img(fg[f].col, fg[f].row, 0, 2) - mean3) * (img(fg[f].col, fg[f].row, 0, 2) - mean3) ;
  }
  
  variance1 = variance1/fg_size;
  variance2 = variance2/fg_size;
  variance3 = variance3/fg_size;

  for(int i=0; i<img.height(); i++){
    for(int j=0; j<img.width(); j++){
      
      double bita = 0;
      
      bita = ((img(j,i,0,0) - mean1)*(img(j,i,0,0) - mean1))/variance1 + ((img(j,i,0,1) - mean2)*(img(j,i,0,1) - mean2))/variance2 + ((img(j,i,0,2) - mean3)*(img(j,i,0,2) - mean3))/variance3 ;
      
//

      if(bita < BETA)
      {
        result(j, i, 0, 0) = 1;
        cost_1(j,i) = bita;
        //cost_0(j,i) = 5;
      }
        
      else
      {
        result(j, i, 0, 0) = 0;
        cost_1(j,i) = bita;
       // cost_0(j,i) = 5;
      }
        
    }
  }
/*      
  for(int f=0; f < fg_size; f++){
    result(fg[f].col, fg[f].row, 0, 0) = 0;
  }
  
  for(int b=0; b < bg_size; b++){
    result(bg[b].col, bg[b].row, 0, 0) = 1;
  }*/

  result.save_png("disp_naive.png");
  return result;
}




double smootheningFn(int l1, int l2)
{
  if(l1==l2)
    return 0;
  else
    return 1;
}



double computeDataCost(int x, int y, int label, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1)
{

  for(int i = 0; i<fg.size(); i++)
  {
      int f_x = fg[i].row;
      int f_y = fg[i].col;

      if(f_x==x && f_y==y)
      {
         if(label == 1)
         {
          return 0;
         }

         if(label == 0)
          return numeric_limits<double>::infinity();
 
      }
  }

  for(int i = 0; i<bg.size(); i++)
  {
    int f_x = bg[i].row;
    int f_y = bg[i].col;

      if(f_x==x && f_y==y)
      {
       
       if(label == 1)
        return numeric_limits<double>::infinity();

        if(label == 0)
          return 0;

      }
  }

  if(label == 1)
    return cost_1(y,x);

  else
    return BETA;
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



double send(int x, int y, int dir, CImg<Pixel> &message_map, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1, int l)
  {
    
    double min_cost = numeric_limits<double>::max();

    for(int label = 0; label<=LABELS; label++)
    {
      double cost = 0;
      cost += computeDataCost(x, y, label, fg, bg, cost_1);


      if(cost < numeric_limits<double>::infinity())
      {

        cost += smootheningFn(l, label);

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

      }
      

      if(min_cost > cost)
        min_cost = cost;

    }

    return min_cost;
  }



int belief(int x, int y, CImg<Pixel> &message_map, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1)
{


  double min_cost = numeric_limits<double>::max();
  int l = 0;

    for(int label = 0; label<=LABELS; label++)
    {
      double cost = 0;
      cost += computeDataCost(x, y, label, fg, bg, cost_1);


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



CImg<double> mrf_segment(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1)
{
  // implement this in step 3...
  //  this placeholder just returns a random disparity map by calling naive_segment
  //return naive_segment(img, fg, bg);

    CImg<Pixel> message_map(img.width(), img.height(),1,1);
    CImg<Pixel> p_message_map(img.width(), img.height(),1,1);

    CImg<double> result(img.width(), img.height(),1,1,0);
    
    for(int itr = 0; itr<ITR; itr++)
    {

      cout << "Iteration: " << itr << endl;

      for(int i = 1; i<img.width()-1; i++)
    {
      for(int j = 1; j<img.height()-1; j++)
      {
        for(int label = 0; label <= LABELS; label++)
        {
          for(int dir = 0; dir<4; dir++)
          {
            
            if(dir == 0 && j-1>=0)
            {
                message_map(i,j-1).msg[label] = send(j,i, dir,p_message_map, fg, bg, cost_1, label);
            }

            if(dir == 1 && i + 1 < img.width())
            {
                message_map(i+1,j).msg[label] = send(j,i, dir,p_message_map, fg, bg, cost_1, label);
            }

            if(dir == 2 && j+1 < img.height())
            {
                message_map(i,j+1).msg[label] = send(j,i, dir,p_message_map, fg, bg, cost_1, label);
            }

            if(dir == 3 && i-1>=0)
            {
                message_map(i-1,j).msg[label] = send(j,i, dir,p_message_map, fg, bg, cost_1, label);
            }


          }
        }
      }

      
    }

      p_message_map = message_map;

    }

    //message_map.save_png("message_map.png");

    //double belief(int x, int y, CImg<Pixel> &message_map, const vector<Point> &fg, const vector<Point> &bg, CImg<double> &cost_1)

    for(int i = 1; i<img.width()-1; i++)
    {
      for(int j = 1; j<img.height()-1; j++)
      {

          result(i,j) = belief(j,i,message_map,fg,bg,cost_1);
        
      }
    }
    

    //result *= 255.0;
    result.save_png("disp_mrf.png");

    return result;
    
  }


// Take in an input image and a binary segmentation map. Use the segmentation map to split the 
//  input image into foreground and background portions, and then save each one as a separate image.
//
// You'll just need to modify this to additionally output a disparity map.
//
void output_segmentation(const CImg<double> &img, const CImg<double> &labels, const string &fname)
{
  // sanity checks. If one of these asserts fails, you've given this function invalid arguments!
  assert(img.height() == labels.height());
  assert(img.width() == labels.width());

  CImg<double> img_fg = img, img_bg = img;

  for(int i=0; i<labels.height(); i++)
    for(int j=0; j<labels.width(); j++)
      {
        
	if(labels(j,i) == 0)
	  img_fg(j,i,0,0) = img_fg(j,i,0,1) = img_fg(j,i,0,2) = 0;
	else if(labels(j,i) == 1)
	  img_bg(j,i,0,0) = img_bg(j,i,0,1) = img_bg(j,i,0,2) = 0;
	else
	  assert(0);
      }

  img_fg.get_normalize(0,255).save((fname + "_fg.png").c_str());
  img_bg.get_normalize(0,255).save((fname + "_bg.png").c_str());
}

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
      cerr << "usage: " << argv[0] << " image_file seeds_file" << endl;
      return 1;
    }

  string input_filename1 = argv[1], input_filename2 = argv[2];

  // read in images and gt
  CImg<double> image_rgb(input_filename1.c_str());
  CImg<double> seeds_rgb(input_filename2.c_str());

  // figure out seed points 
  vector<Point> fg_pixels, bg_pixels;
  for(int i=0; i<seeds_rgb.height(); i++)
    for(int j=0; j<seeds_rgb.width(); j++)
      {
	// blue --> foreground
	if(max(seeds_rgb(j, i, 0, 0), seeds_rgb(j, i, 0, 1)) < 100 && seeds_rgb(j, i, 0, 2) > 100)
	  fg_pixels.push_back(Point(j, i));

	// red --> background
	if(max(seeds_rgb(j, i, 0, 2), seeds_rgb(j, i, 0, 1)) < 100 && seeds_rgb(j, i, 0, 0) > 100)
	  bg_pixels.push_back(Point(j, i));
      }

  // do naive segmentation
  CImg<double> cost_0(image_rgb.width(),image_rgb.height());
  CImg<double> cost_1(image_rgb.width(),image_rgb.height());

  CImg<double> labels = naive_segment(image_rgb, fg_pixels, bg_pixels, cost_1);
  output_segmentation(image_rgb, labels, input_filename1 + "-naive_segment_result");

  labels *= 255.0;
  labels.save_png("part_2_1_disp.png");

  // do mrf segmentation
  labels = mrf_segment(image_rgb, fg_pixels, bg_pixels, cost_1);
  output_segmentation(image_rgb, labels, input_filename1 + "-mrf_segment_result");

  labels *= 255.0;
  labels.save_png("lbp.png");

  return 0;
}
