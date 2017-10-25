// B657 assignment 2 skeleton code
//
// Compile with: "make"
//
// See assignment handout for command line and project specifications.

//Compiling steps
//For part 1.1 compile using make followed by ./a2 part1.1 bigben_2.jpg bigben_3.jpg matched.jpg
//For part 1.2 compile using make followed by ./a2 part1.2 bigben_2.jpg bigben_3.jpg colosseum_8.jpg colosseum_12.jpg
//For part 1.3 compile using make followed by ./a2 part1.3
//For part 1.4 compile using make followed by ./a2 part1.4
//For part 2.1 and 2.3 compile using make followed by ./a2 part2 img_1.png img_2.png ... img_n.png

//Link to the header file
#include "CImg.h"
#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Sift.h>
#include <algorithm>
#include <random>
#include<dirent.h>
#include <map>
#include <fstream>
#include <math.h>
#include <time.h>

#define SIZE 28

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;


//Function declarations Part 1 Q1 and Q2

CImg<double> SIFT_match(CImg<double> ,CImg<double>, string);


int descriptor_count = 0;
class image{
	public: string name;
	        double desc;
};

//Used to compare images in 1.2
struct compare {
    bool operator ()(image const *a, image const *b) const {
        
        return (a->desc > b->desc);
    }
};

//Used to retrieve the closest and the second closest match
struct compare1 {
    bool operator ()(double a, double b){
	return (a<b);
    }
};

//Function declarations Part 1 Q3 and Q4

vector<string> Images_in_folder(const char* dir_name);
vector<string> Random_images(const char* dir_name);

vector<vector<double> > gausian_vectors(int, int);
double random_normal (double, double);

double dot_product(vector<double>, SiftDescriptor);
vector<int> summarization_function(vector<vector<double> >, vector<SiftDescriptor>, int, int);
vector<vector<double> > Distance_values(vector<vector<double> >, vector<SiftDescriptor>, vector<SiftDescriptor>);

int Image_Match_3(CImg<double>, CImg<double>);
int Image_Match_4(CImg<double>, CImg<double>);

double Precision(string, vector<string>, string);




// This class is used to track the matching SIFT descriptor coordinates between 2 images
class FeatureCorrespondence{

	public:
		double x1; //Image 1 SIFT descriptor column coordinate
		double y1; //Image 1 SIFT descriptor row coordinate
		double x2; //Image 2 SIFT descriptor column coordinate
		double y2; //Image 2 SIFT descriptor row coordinate
};

// Function declarations for Part 1 Q1 and Q2


// For Part 1 Q1 and Q2 compute the SIFT descriptors between 2 images
CImg<double> SIFT_match(CImg<double> first, CImg<double> second,string part){
		
	double color[] = { 255,0,0 }, num_of_desc = 0;			
	double sum = 0, val = 0, x1, y1, x2, y2; 
	CImg<double> output(first,"xyzc",0);
	double max_height = max(first.height(),second.height());
		
	output.resize(first.width()+second.width(),max_height);
	
	for(int i = 0; i < first.width() + second.width(); i++){
		for(int j = 0;j < max_height; j++){
			if(i < first.width() && j < first.height()){
				if(first.spectrum() == 3){
					output(i,j,0,0) = first(i,j,0,0);
					output(i,j,0,1) = first(i,j,0,1);
					output(i,j,0,2) = first(i,j,0,2);
				}
				else if(first.spectrum() == 1){
					output(i,j,0) = first(i,j,0);
				}
			}
		}
	}
	
	for(int i = 0; i < first.width() + second.width(); i++){
		for(int j = 0;j < max_height; j++){
			if(i < first.width() && j >= first.height()){
				output(i,j,0,0) = 0;
				output(i,j,0,1) = 0;
				output(i,j,0,2) = 0;
			}
		}
	}
	
	for(int i = 0; i < first.width() + second.width(); i++){
		for(int j = 0;j < max_height; j++){
			if(i >= first.width() && j < second.height()){
				if(second.spectrum() == 3){
					output(i,j,0,0) = second(i - first.width(),j,0,0);
					output(i,j,0,1) = second(i - first.width(),j,0,1);
					output(i,j,0,2) = second(i - first.width(),j,0,2);
				}
				else if(second.spectrum() == 1){
					output(i,j,0) = second(i - first.width(),j,0);
				}	
			}
		}
	}
	
	
	
	vector<SiftDescriptor> descriptors, descriptors2;
	vector<double> value1;
	
		if(first.spectrum() != 1){
			CImg<double> gray = first.get_RGBtoHSI().get_channel(2);
			descriptors = Sift::compute_sift(gray);
		}
		
		if(second.spectrum() != 1){
			CImg<double> gray2 = second.get_RGBtoHSI().get_channel(2);
			descriptors2 = Sift::compute_sift(gray2);
		}
		
		for(int i=0; i<descriptors.size(); i++){
			//cout<<"Inside the i loop"<<endl;
			double final_j=0;
			double min1=800;
			double min2=1000;
		   	for(int j=0; j<descriptors2.size(); j++){
					
				sum = 0;
				
				for(int l=0; l<128; l++){
					sum += pow(descriptors[i].descriptor[l] - descriptors2[j].descriptor[l],2); 	
				}
						
				val = sqrt(sum);
				if( val < min1){
					min2 = min1;
					min1 = val;
					final_j=j;
				}
				else if( val >= min1 && val < min2){
					min2 = val;
				}
			}
				double final=min1/min2;
					if(final < 0.8 ){	
					num_of_desc++;
							
					x1 = descriptors[i].col;     x2 = first.width() + descriptors2[final_j].col;
					y1 = descriptors[i].row;     y2 = descriptors2[final_j].row;
					
					        
					output.draw_line(x1,y1,x2,y2,color);
				}
        }
	
	
        
        if(part == "part1.1"){
        	cout<<"Number of matching descriptors are: "<<num_of_desc<<endl;
        	output.get_normalize(0,255);
			return output;
		}
		else if(part == "part1.2"){
			
			descriptor_count = num_of_desc;
			
		}
		
}

// Function declarations for Part 1 Q3 and Q4

int Image_Match_3(CImg<double> input, CImg<double> input2){

    int num_of_desc = 0;			
	double sum = 0, val = 0;


	vector<SiftDescriptor> descriptors, descriptors2;
	vector<double> value1;
	
		if(input.spectrum() != 1){
			CImg<double> gray = input.get_RGBtoHSI().get_channel(2);
			descriptors = Sift::compute_sift(gray);
		}
		
		if(input2.spectrum() != 1){
			CImg<double> gray2 = input2.get_RGBtoHSI().get_channel(2);
			descriptors2 = Sift::compute_sift(gray2);
		}
        
        int count = 0;
		
		for(int i=0; i<descriptors.size(); i++){

			double min1=800;
			double min2=1000;
		   	for(int j=0; j<descriptors2.size(); j++){
                   
                count++;
					
				sum = 0;
				
				for(int l=0; l<128; l++){
					sum += pow(descriptors[i].descriptor[l] - descriptors2[j].descriptor[l],2); 	
				}
						
				val = sqrt(sum);
				if( val < min1){
					min2 = min1;
					min1 = val;
				}
				else if( val >= min1 && val < min2){
					min2 = val;
				}
			}
				double final=min1/min2;
				//cout<<"Final is:"<<final<<endl;
				if(final < 0.8 ){	
					num_of_desc++;
				}
				
			//}
			
			
        }
//         cout<<"loop count in 1 = "<<count<<endl;
			
		return num_of_desc;
			
}

vector<string> Images_in_folder(const char* dir_name = "part1_images"){
    
    vector<string> list, images_string;
    struct dirent *pDirent;
    DIR *pDir = opendir (dir_name);
  
    if (pDir == NULL) {
        printf ("Cannot open directory\n");
        //return 1;
    }

    while ((pDirent = readdir(pDir)) != NULL) {
        //if(!(pDirent->d_name == "." || pDirent->d_name == "..")) - not working!!!???
        list.push_back(pDirent->d_name);
    }
    
    sort(list.begin(), list.end());
    
    for(int i=2; i < list.size(); i++)
        images_string.push_back(list[i]);
    
    closedir (pDir);
        
    return images_string;
}

vector<string> Random_images(const char* dir_name = "part1_images"){

  int num;
  vector<string> random_images;
  
  vector<string> part1_images = Images_in_folder(dir_name);
  
  // construct a trivial random generator engine from a time-based seed: 
  //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  
  unsigned seed = time(NULL);

  srand(seed);  
  for(int i=0; i < part1_images.size(); i = i + 10){     
      
      num = rand() % 10;
      
      //cout<< num << " " << part1_images[i + num] << endl;
      random_images.push_back(part1_images[i + num]);
  }
  
  return random_images;
    
}

double random_normal (double mu, double sigma){
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;
 
  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }
 
  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);
 
  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;
 
  call = !call;
 
  return (mu + sigma * (double) X1);
}


vector<vector<double> > gausian_vectors(int size = 128, int length = SIZE){
  
  vector<vector<double> > x;

  x.resize(length);
  for (int i = 0; i < length; ++i)
    x[i].resize(size);
  
  for (int i=0; i<length; ++i){
      for (int j=0; j<size; ++j)
          x[i][j]= random_normal(0.0, 1.0);
  }
  
  return x;
}

double dot_product(vector<double> x, SiftDescriptor sift_vector){
    double sum = 0.0;
    for(int i=0; i < x.size(); i++)
        sum += x[i]*sift_vector.descriptor[i];
    return sum;
}

vector<int> summarization_function(vector<vector<double> > x, SiftDescriptor sift_vector, int w, int length = SIZE){
    
    vector<int> f;
    f.resize(length);
    
    for(int i=0; i<length; i++){
        f[i] = (int)round(dot_product(x[i], sift_vector)) / w;   
    }    
    
    return f;
}

//for problem 1-4


vector<vector<double> > Distance_values(vector<vector<double> > x, vector<SiftDescriptor> descriptors_1, vector<SiftDescriptor> descriptors_2){
    
    //cout<<"descriptors_1 size = "<<descriptors_1.size()<<endl;
    //cout<<"descriptors_2 size = "<<descriptors_2.size()<<endl;
    
    vector<vector<int> > descriptors_1f, descriptors_2f;
    
    descriptors_1f.resize(descriptors_1.size());
    descriptors_2f.resize(descriptors_2.size());
            
    for(int p=0; p<descriptors_1.size(); p++){
        descriptors_1f[p] = summarization_function(x, descriptors_1[p], 32, SIZE);
    }
    
	for(int q=0; q<descriptors_2.size(); q++){  
        descriptors_2f[q] = summarization_function(x, descriptors_2[q], 32, SIZE);
    }
    
    vector<vector<double> > distance_vals;
    
    distance_vals.resize(descriptors_1.size());
    
    for(int d=0; d<descriptors_1.size(); d++)
        distance_vals[d].resize(descriptors_2.size());
    
	for(int r=0; r<descriptors_1.size(); r++){
        double dist = 0;	
		for(int s=0; s<descriptors_2.size(); s++){
            for(int z=0; z<SIZE; z++)              
                dist += pow(descriptors_1f[r][z] - descriptors_2f[s][z], 2);	

            distance_vals[r][s] = sqrt(dist);
		}
    }
    
    return distance_vals;
}


int Image_Match_4(CImg<double> input, CImg<double> input2){
    
    vector<SiftDescriptor> descriptors, descriptors2;
    int num_of_desc = 0;
    
    if(input.spectrum() != 1){
		CImg<double> gray = input.get_RGBtoHSI().get_channel(2);
		descriptors = Sift::compute_sift(gray);
	}
		
	if(input2.spectrum() != 1){
		CImg<double> gray2 = input2.get_RGBtoHSI().get_channel(2);
		descriptors2 = Sift::compute_sift(gray2);
	}
    
    vector<vector<double> > x = gausian_vectors(128, SIZE);
    
    vector<vector<double> > distance_values = Distance_values(x, descriptors, descriptors2);
    
    int sum = 0, val = 0; 
    
    int count = 0;
    
    for(int i=0; i<descriptors.size(); i++){  

		double min1=750;
		double min2=850;
        
		for(int j=0; j<descriptors2.size(); j++){

            if(distance_values[i][j] < 500){
                
                //cout<<distance_values[i][j]<<endl;
                
                count++;

			    sum = 0;

			    for(int l=0; l<128; l++){
                    sum += pow(descriptors[i].descriptor[l] - descriptors2[j].descriptor[l],2); 	
			    }
    
                val = sqrt(sum);
                //cout<<val<<endl;
	            if( val < min1){
		            min2 = min1;
			        min1 = val;
		        }
                else if( val >= min1 && val < min2){
			        min2 = val;
		        }
                
                double final = min1/min2;
                
		        if(final < 0.45 )	
	               num_of_desc++;
		    }

        }

    }
    
    //cout<<"loop count in 2 = "<<count<<endl;
    			
	return num_of_desc;   
}

double Precision(string random_image, vector<string> part1_images, string part){
    
    int match;
        
    //this is used to map image to number of sift discriptior matches
    map<int, string> matching;
        
    string folder = "a2-images/part1_images/";
        
    //a randomly choosed image
    CImg<double> input((folder + random_image).c_str());        
        
    //all images in the folder
    for(int j=0; j<part1_images.size(); j++){
            
        CImg<double> input2((folder + part1_images[j]).c_str());
        
        if(part == "part1.3")    
            match = Image_Match_3(input, input2);  //for part 1.3 normal sift
        else if(part == "part1.4")
            match = Image_Match_4(input, input2);  //for part 1.4 modified sift
        
        //calculate the number of discriptors that are matching between random image to the
        //images in folder and map those values using the map "matching"
        matching.insert(pair<int, string>(match, part1_images[j]));	
    }
 
    int sum = 0;
    double precision;
    
    cout<<"--------------------------------------------------------------"<<endl;
    cout<<"Top ten matched images"<<endl;  
     
    int count = 0;
    
    for(map<int, string>::const_reverse_iterator it = matching.rbegin(); it != matching.rend(); ++it){
          
        if(count < 11){
            count++;
            //cout<<it -> second<< endl;
            string s = it -> second;

            cout<<count<<"   "<<s<<endl;
            
            if(random_image[0] == s[0] && random_image[1] == s[1] && random_image[2] == s[2])
                sum = sum + 1;                
        }
    }
        
    precision = sum/0.1;
    
    return precision;
    
}



// Function declarations for Part 2 Q1 , Q2 and Q3


// For Part 2 Q2 and Q3 this method returns a vector containing the matched SIFT pair co-ordinates between the 2 input images
vector<FeatureCorrespondence> generateSampleSpace(CImg<double> image1, CImg<double> image2){
	
	int sum;
	double val;
	vector<SiftDescriptor> image1Descriptor, image2Descriptor;
	vector<FeatureCorrespondence> sampleSpace;
		if(image1.spectrum() != 1){
			CImg<double> gray = image1.get_RGBtoHSI().get_channel(2);
			image1Descriptor = Sift::compute_sift(gray);
		}
		
		if(image2.spectrum() != 1){
			CImg<double> gray2 = image2.get_RGBtoHSI().get_channel(2);
			image2Descriptor = Sift::compute_sift(gray2);
		}
		double numberOfValidDesciptors=0;
		for(int i=0; i<image1Descriptor.size(); i++){
			
			double final_j=0;
			double min1=800;
			double min2=1000;
			
		   	for(int j=0; j<image2Descriptor.size(); j++){
					
				sum = 0;
				
				for(int l=0; l<128; l++){
					sum += pow(image1Descriptor[i].descriptor[l] - image2Descriptor[j].descriptor[l],2); 	
				}
						
				val = sqrt(sum);
				if( val < min1){
					min2 = min1;
					min1 = val;
					final_j=j;
				}
				else if( val >= min1 && val < min2){
					min2 = val;
				}
			}
				double final=min1/min2;
					if(final < 0.65){
					numberOfValidDesciptors++;
					
					FeatureCorrespondence matchingFeaturePoints;
					matchingFeaturePoints.x1 = image1Descriptor[i].col;
					matchingFeaturePoints.y1 = image1Descriptor[i].row;	
					matchingFeaturePoints.x2 = image2Descriptor[final_j].col;
					matchingFeaturePoints.y2 = image2Descriptor[final_j].row;
					
					sampleSpace.push_back(matchingFeaturePoints);
				}
			}
			
			return sampleSpace;
        }
	


// Method to return the transformation matrix from 4 random pairs of matching SIFT descriptor points between 2 images
CImg<double> getTransformationMatrix(FeatureCorrespondence featurePair1,FeatureCorrespondence featurePair2,FeatureCorrespondence featurePair3,FeatureCorrespondence featurePair4){

			
			// Prepare Matrix A
			CImg<double> A(8,8);
			double x1=featurePair1.x1;
			double y1=featurePair1.y1;
			double x1_dash=featurePair1.x2;
			double y1_dash=featurePair1.y2;
			double x2=featurePair2.x1;
			double y2=featurePair2.y1;
			double x2_dash=featurePair2.x2;
			double y2_dash=featurePair2.y2;
			double x3=featurePair3.x1;
			double y3=featurePair3.y1;
			double x3_dash=featurePair3.x2;
			double y3_dash=featurePair1.y2;
			double x4=featurePair4.x1;
			double y4=featurePair4.y1;
			double x4_dash=featurePair4.x2;
			double y4_dash=featurePair4.y2;
			
			//First Row
			A(0,0)=x1;
			A(1,0)=y1;
			A(2,0)=1;
			A(3,0)=0;
			A(4,0)=0;
			A(5,0)=0;
			A(6,0)=-(x1*x1_dash);
			A(7,0)=-(y1*x1_dash);

			//Second Row
			A(0,1)=0;
			A(1,1)=0;
			A(2,1)=0;
			A(3,1)=x1;
			A(4,1)=y1;
			A(5,1)=1;
			A(6,1)=-(x1*y1_dash);
			A(7,1)=-(y1*y1_dash);

			//Third Row
			A(0,2)=x2;
			A(1,2)=y2;
			A(2,2)=1;
			A(3,2)=0;
			A(4,2)=0;
			A(5,2)=0;
			A(6,2)=-(x2*x2_dash);
			A(7,2)=-(y2*x2_dash);

			//Fourth Row
			A(0,3)=0;
			A(1,3)=0;
			A(2,3)=0;	
			A(3,3)=x2;
			A(4,3)=y2;
			A(5,3)=1;
			A(6,3)=-(x2*y2_dash);
			A(7,3)=-(y2*y2_dash);

			//Fifth Row
			A(0,4)=x3;
			A(1,4)=y3;
			A(2,4)=1;
			A(3,4)=0;
			A(4,4)=0;
			A(5,4)=0;
			A(6,4)=-(x3*x3_dash);
			A(7,4)=-(y3*x3_dash);

			//Sixth Row
			A(0,5)=0;
			A(1,5)=0;
			A(2,5)=0;
			A(3,5)=x3;
			A(4,5)=y3;
			A(5,5)=1;
			A(6,5)=-(x3*y3_dash);
			A(7,5)=-(y3*y3_dash);
			
			//Seventh Row
			A(0,6)=x4;
			A(1,6)=y4;
			A(2,6)=1;
			A(3,6)=0;
			A(4,6)=0;
			A(5,6)=0;
			A(6,6)=-(x4*x4_dash);
			A(7,6)=-(y4*x4_dash);

			//Eighth Row
			A(0,7)=0;
			A(1,7)=0;
			A(2,7)=0;
			A(3,7)=x4;
			A(4,7)=y4;
			A(5,7)=1;
			A(6,7)=-(x4*y4_dash);
			A(7,7)=-(y4*y4_dash);

			//Prepare Matrix B
			CImg<double> B(1,8);
			B(0,0)=x1_dash;
			B(0,1)=y1_dash;
			B(0,2)=x2_dash;
			B(0,3)=y2_dash;
			B(0,4)=x3_dash;
			B(0,5)=y3_dash;
			B(0,6)=x4_dash;
			B(0,7)=y4_dash;
			
			// Linear equation : AX=B where X is the transformation matrix

			CImg<double> X=B.solve(A);
			return X;

}

//generate Best Transformation Matirx using RANSAC
CImg<double> generateModel(CImg<double> image1, CImg<double> image2){

	vector<FeatureCorrespondence> sampleSpace=generateSampleSpace(image1,image2);
	
	
	int NO_OF_ITERATIONS=50000;
	int ERROR_THRESHOLD=10;
	CImg<double> bestModel(1,8);
	
	FeatureCorrespondence featurePair1;
	FeatureCorrespondence featurePair2;
	FeatureCorrespondence featurePair3;
	FeatureCorrespondence featurePair4;
	CImg<double> model;
	double x1,x2,y1,y2,u,v,w;
	int numberOfInliers;
	int maxInlierCount=-1;
	

	for(int i=0;i<NO_OF_ITERATIONS;i++){
		std::srand ( unsigned (std::time(0)));
		std::random_shuffle ( sampleSpace.begin(), sampleSpace.end() );
		// Now we have 4 pairs of corresponding features
		featurePair1=sampleSpace.at(0);
		featurePair2=sampleSpace.at(1);
		featurePair3=sampleSpace.at(2);
		featurePair4=sampleSpace.at(3);
	
		model=getTransformationMatrix(featurePair1,featurePair2,featurePair3,featurePair4);
		numberOfInliers=0;
		for(int j=4;j<sampleSpace.size();j++){
			x1=sampleSpace.at(j).x1;
			x2=sampleSpace.at(j).x2;
			y1=sampleSpace.at(j).y1;
			y2=sampleSpace.at(j).y2;
			
			u=model(0,0)*x1+model(0,1)*y1+model(0,2)*1;
			v=model(0,3)*x1+model(0,4)*y1+model(0,5)*1;
			w=model(0,6)*x1+model(0,7)*y1+1;
			u=u/w;
			v=v/w;
			
			//Absolute difference between X and Y coordinate approach
			if(abs(u-x2)<=ERROR_THRESHOLD and abs(v-y2)<=ERROR_THRESHOLD){
 				numberOfInliers++;
 			}
			
			//Euclidean distance approach
			// if(sqrt(pow(u-x2,2)+pow(v-y2,2))<=ERROR_THRESHOLD){
// 				numberOfInliers++;
// 			}
		
		}
		if (numberOfInliers>maxInlierCount){
			
			maxInlierCount=numberOfInliers;
			bestModel=model;
		}
	
	
	}
  
 return bestModel;


}




//Warp a given image based on the transoframtion matrix
void warpImage(CImg<double> input_image,CImg<double> transformationMatrix,string outputFileName){

	int cols=input_image.width();
	int rows=input_image.height();
	
	CImg<double> output_image(cols,rows,1,3,255);  //Initialize an output image with white pixels
	
	

	
	//Inverse Warping code
	//[x][y] are the cooridnates of the output_image
	//[u_dash][v_dash] are the coordinates of the input_image
	int w=1;
	double u;
	double v;
	double w_dash;
	int u_dash;
	int v_dash;
	for(int x=0;x<cols;x++){
		for(int y=0;y<rows;y++){
		
		//Computing the corresponding coordinates in the source image
		
		
		u=transformationMatrix(0,0) * x+transformationMatrix(1,0) * y +transformationMatrix(2,0) * 1;
		v=transformationMatrix(0,1) * x+transformationMatrix(1,1) * y+transformationMatrix(2,1) * 1;
		w_dash=transformationMatrix(0,2) * x+transformationMatrix(1,2) * y+transformationMatrix(2,2) * 1;
		u_dash=u/w_dash;
		v_dash=v/w_dash;
		 
		
		// Assinging pixel values to output image from input image
		if(u_dash>0 && v_dash>0 && u_dash<cols && v_dash<rows){
			output_image(x,y,0,0)=input_image(u_dash,v_dash,0,0);
			output_image(x,y,0,1)=input_image(u_dash,v_dash,0,1);
			output_image(x,y,0,2)=input_image(u_dash,v_dash,0,2);
		
		}
		
		}
		
	
	}
	output_image.save(outputFileName.c_str());


}





int main(int argc, char **argv)
{
  try {

    if(argc < 2)
      {
	cout << "Insufficent number of arguments; correct usage:" << endl;
	cout << "    a2-p1 part_id ..." << endl;
	return -1;
      }

    string part = argv[1];
    
    if(part == "part1.1")
     {
      	if(argc != 5) { 
      	
      		cout<<"Error: Invalid number of arguments"<<endl; 
      		return false;
     }
    	
    char* firstFile = argv[2];
	char* firstFile2 = argv[3];
	char* outputFile = argv[4];
		
	CImg<double> first(firstFile);
	CImg<double> second(firstFile2);
			
	CImg<double> output = SIFT_match(first,second,part);	
	output.save(outputFile);
		
     }
	 else if(part == "part1.2")
     {
     	if(argc < 4) { cout<<"Error: Invalid number of arguments"<<endl; return false;}
     	
     	char* firstFile = argv[2];
     	CImg<double> first(firstFile);
     	
     	vector<image*> vec;
     	image* im;
     	
	//To include multiple images for comparison with a particular image
     	for(int i = 3; i < argc; i++){
     		 
     		 im = new image();
     		 im->name = argv[i];
     		 
     		char* firstFile2 = argv[i];	 
     		CImg<double> second(firstFile2);
     		
     		SIFT_match(first,second,part);	
     		
     		im->desc = descriptor_count;
     		vec.push_back(im);
     		
     	}
     	vector<image*> :: iterator images;
     	
	
     	sort(vec.begin(),vec.end(),compare());
     	
     	for(images = vec.begin(); images != vec.end(); images++){
     		cout<<(*images)->name<<"     ";
     		cout<<(*images)->desc<<endl;
     		
     	}
     }
     
     else if(part == "part1.3")
    {
         vector<string> part1_images = Images_in_folder("a2-images/part1_images");
         vector<string> random_images = Random_images("a2-images/part1_images");
             
         cout<<"_____________________________random images_____________________________"<<endl;         
         for(int i=0; i<10; i++)
             cout<<random_images[i]<<endl; 
         cout<<"-----------------------------------------------------------------------"<<endl;                           
                      
         cout<<"bigben"<<" precision            "<<Precision(random_images[0], part1_images, part)<<endl;
         cout<<"colosseum"<<" precision         "<<Precision(random_images[1], part1_images, part)<<endl;
         cout<<"eiffel"<<" precision            "<<Precision(random_images[2], part1_images, part)<<endl;
         cout<<"empirestate"<<" precision       "<<Precision(random_images[3], part1_images, part)<<endl;
         cout<<"londoneye"<<" precision         "<<Precision(random_images[4], part1_images, part)<<endl;
         cout<<"louvre"<<" precision            "<<Precision(random_images[5], part1_images, part)<<endl;
         cout<<"notredame"<<" precision         "<<Precision(random_images[6], part1_images, part)<<endl;
         cout<<"sanmarco"<<" precision          "<<Precision(random_images[7], part1_images, part)<<endl;
         cout<<"tatemodern"<<" precision        "<<Precision(random_images[8], part1_images, part)<<endl;  
         cout<<"trafalgarsquare"<<" precision   "<<Precision(random_images[9], part1_images, part)<<endl;
     } 
    
        
    else if(part == "part1.4")
    {
         
         clock_t t1,t2;
         
         vector<string> part1_images = Images_in_folder("a2-images/part1_images");
         vector<string> random_images = Random_images("a2-images/part1_images");
         
         t1=clock();
         cout<<"the precision of chosen random image "<<random_images[1]<<"  "<<Precision(random_images[1], part1_images, part)<<endl;
         t2=clock();
         float diff ((float)t2-(float)t1);
//          cout<<"time taken to run image matching of 1 image among 100 "<<diff<<endl;
     }     

    else if(part == "part2")
      {
	
	/*
	###################
	#PART 2 Question 1#
	###################
	*/
		CImg<double> input_image1(argv[2]);
		
		//Preparing the file name of the warped image
		string fileName=basename(argv[2]);
		string file=fileName.substr(0,fileName.find("."))+"-warped";
		string extension=fileName.substr(fileName.find("."),fileName.length());
		string outputFileName=file+extension;
		
		//Initializing the matrix as per the values in the assignment
		CImg<double> transformationMatrix(3,3);
		transformationMatrix(0,0)=0.907;
		transformationMatrix(1,0)=0.258;
		transformationMatrix(2,0)=-182;
		transformationMatrix(0,1)=-0.153;
		transformationMatrix(1,1)=1.44;
		transformationMatrix(2,1)=58;
		transformationMatrix(0,2)=-0.000306;
		transformationMatrix(1,2)=0.000731;
		transformationMatrix(2,2)=1;
		//Invert the matrix for Reverse Warping
		transformationMatrix=transformationMatrix.invert();
		
		warpImage(input_image1,transformationMatrix,outputFileName);

	/*
	##################################
	#PART 2 Question 2 and Question 2#
	##################################
	*/
	
	
	for(int i=3;i<argc;i++){
			CImg<double> input_image2(argv[i]);
			
			//Preparing the file name of the warped image	
			fileName=basename(argv[i]);
			file=fileName.substr(0,fileName.find("."))+"-warped";
			extension=fileName.substr(fileName.find("."),fileName.length());
			outputFileName=file+extension;
	
	
			CImg<double> X=generateModel(input_image1,input_image2); // a column vector with 8 rows
			CImg<double> X_transformed(3,3); //transforming the column vector into 3x3 matrix
			X_transformed(0,0)=X(0,0);
			X_transformed(1,0)=X(0,1);
			X_transformed(2,0)=X(0,2);
			X_transformed(0,1)=X(0,3);
			X_transformed(1,1)=X(0,4);
			X_transformed(2,1)=X(0,5);
			X_transformed(0,2)=X(0,6);
			X_transformed(1,2)=X(0,7);
			X_transformed(2,2)=1;
		
			//Inverse the generated projective transformation matrix and warp the image
			CImg<double> X_Inverse=X_transformed.invert();
			
			warpImage(input_image2,X_Inverse,outputFileName);
	
	}
	
      }
    else
      throw std::string("unknown part!");

    // feel free to add more conditions for other parts (e.g. more specific)
    //  parts, for debugging, etc.
  }
  catch(const string &err) {
    cerr << "Error: " << err << endl;
  }
}








