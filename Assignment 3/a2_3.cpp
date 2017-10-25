// B657 assignment 3 skeleton code, D. Crandall
//
// Compile with: "make"
//
// This skeleton code implements nearest-neighbor classification
// using just matching on raw pixel values, but on subsampled "tiny images" of
// e.g. 20x20 pixels.
//
// It defines an abstract Classifier class, so that all you have to do
// :) to write a new algorithm is to derive a new class from
// Classifier containing your algorithm-specific code
// (i.e. load_model(), train(), and classify() methods) -- see
// NearestNeighbor.h for a prototype.  So in theory, you really
// shouldn't have to modify the code below or the code in Classifier.h
// at all, besides adding an #include and updating the "if" statement
// that checks "algo" below.
//
// See assignment handout for command line and project specifications.
//
#include "CImg.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Sift.h>
#include <sys/types.h>
#include <dirent.h>
#include <map>
#include <numeric>
#include <algorithm>
#include <cmath>
//#include "bow_model.h"


#include </usr/include/opencv2/highgui/highgui.hpp>
#include </usr/include/opencv2/core/core.hpp>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;
using namespace cv;

// Dataset data structure, set up so that e.g. dataset["bagel"][3] is
// filename of 4th bagel image in the dataset
typedef map<string, vector<string> > Dataset;

#include <Classifier.h>
#include <NearestNeighbor.h>

// Figure out a list of files in a given directory.
//
vector<string> files_in_directory(const string &directory, bool prepend_directory = false){
  vector<string> file_list;
  DIR *dir = opendir(directory.c_str());
  if(!dir)
    throw std::string("Can't find directory " + directory);
  
  struct dirent *dirent;
  while ((dirent = readdir(dir))) 
    if(dirent->d_name[0] != '.')
      file_list.push_back((prepend_directory?(directory+"/"):"")+dirent->d_name);

  closedir(dir);
  return file_list;
}


//converts cv::Mat(CV_32F) to vector<vector<float> > format
vector<vector<float> > Mat_to_vector(Mat A){
    
    vector<vector<float> > x(A.rows);
    for(int i=0; i<A.rows; i++)
        x[i].resize(A.cols);
        
    for(int r=0; r<A.rows; r++){
        for(int c=0; c<A.cols; c++){
            x[r][c] = A.at<float>(r,c);
        }
    }
    
    return x;

}

//converts vector<SiftDescriptor> format to vector<vector<float> >
vector<vector<float> > SiftDescriptor_to_vector(vector<SiftDescriptor> A){
    
    vector<vector<float> > x(A.size());
    for(int i=0; i<A.size(); i++)
        x[i].resize(128);
        
    for(int r=0; r<A.size(); r++){
        for(int c=0; c<128; c++){
            x[r][c] = A[r].descriptor[c];
        }
    }
    
    return x;

}

//centers = cluster centers, descriptors 
//takes cluster centers of kmeans model and descriptors of an image and generates bag of words histogram for that image 
vector<int> bow_histogram_calculation(vector<vector<float> > centers, vector<vector<float> > descriptors, int clusterCount){
    
    vector<int> bow_histogram(clusterCount);
    for(int h=0; h<clusterCount; h++){
        bow_histogram[h] = 0;
    }

    
    for(int a=0; a<descriptors.size(); a++){
        
        vector<double> euclidian_distances(0);
        
        for(int b=0; b<centers.size(); b++){
            
            double distance_bw = 0;
            
            for(int x=0; x<128; x++){
                distance_bw = distance_bw + pow( ( descriptors[a][x] - centers[b][x] ), 2);
            }
            euclidian_distances.push_back(distance_bw);
        }
        
        vector<double>::iterator result = min_element(euclidian_distances.begin(), euclidian_distances.end());
        //cout << "min element at: " << distance(begin(euclidian_distances), result);
        
        bow_histogram[distance(euclidian_distances.begin(), result)]++;
        
    }
    
    return bow_histogram;
} 

//filenames = folder name, size = the size to which images in folder are resized to, clusterCount = number of clusters in kmeans
//takes folder filename, size and clusterCount and return the centers of kmeans clusters
//also saves cluster centers in a centers.yml
Mat kmeans_clusters_centers(const Dataset &filenames, int size = 100, int clusterCount = 500){
    
	vector<SiftDescriptor> descriptors_final;
	vector<SiftDescriptor> descriptors;
        
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter){
                
        cout << "Processing " << c_iter->first << endl;
        
        for(int i=0; i<c_iter->second.size(); i++){

            CImg<double> image(c_iter->second[i].c_str());
            
            CImg<double> resized_image = image.resize(size,size,1,3);
	
		    if(resized_image.spectrum() != 1){
			    CImg<double> gray = resized_image.get_RGBtoHSI().get_channel(2);
			    descriptors = Sift::compute_sift(gray);
                }
                
            //cout<<c_iter->second[i].c_str()<<"  "<<descriptors.size()<<endl;
                            
            descriptors_final.insert( descriptors_final.end(), descriptors.begin(), descriptors.end() );

        }
            
    }
    
    cout<<"total number of descriptors "<<descriptors_final.size()<<endl;

    int sampleCount = descriptors_final.size(); //total number of sift descriptors for all the images in train folder
    int dimensions = 128;   //sift dimention
    //int clusterCount = 500;  //number of words in the histogram
    int attempts = 100;
       
    Mat train_images_sift_descriptors(sampleCount, dimensions, CV_32F);
    Mat dictionary(sampleCount, 1, CV_32F);
    Mat centers(clusterCount, 128, CV_32F);
    
    for(int i=0; i < sampleCount; i++){
        for(int j=0; j < dimensions; j++){
            train_images_sift_descriptors.at<float>(i,j) = descriptors_final[i].descriptor[j];
            }
    }
    
    kmeans(train_images_sift_descriptors, clusterCount, dictionary, TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 1000, .001), attempts, KMEANS_PP_CENTERS, centers);    

    FileStorage fsd("dictionary_40.yml", FileStorage::WRITE);
    fsd << "vocabulary" << dictionary;
    fsd.release();
    
    FileStorage fsc("centers_40.yml", FileStorage::WRITE);
    fsc << "clusters" << centers;
    fsc.release();
    
    return centers;

}

//filenames = folder name, size = the size to which images in folder are resized to, clusterCount = number of clusters in kmeans
//takes a folder filename, size and clusterCount and generates bow histograms for all images in folder
//uses centers.yml file generated using "kmeans_clusters_centers" function
vector<vector<int> > bow_histograms_for_all_images_in_folder(const Dataset &filenames, int size = 100, int clusterCount = 500){
    
    Mat centers; 
    FileStorage fs("centers_40.yml", FileStorage::READ);
    fs["clusters"] >> centers;
    fs.release();
    
    //int clusterCount = centers.rows;
    
    //a vector containing histograms of all images in train folder
    vector<vector<int> > bow_histograms_vector;
    
    //converting centers of clusters we got in kmeans from Mat format to vector<vector<float> > format
    vector<vector<float> > centers_v = Mat_to_vector(centers);

	vector<SiftDescriptor> descriptors;
    
    int hist = 0;
        
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter){
                
        //cout << "--------------------------------Processing " << c_iter->first << " folder------------------------------" <<endl;
        
        for(int i=0; i<c_iter->second.size(); i++){

            CImg<double> image(c_iter->second[i].c_str());
            
            CImg<double> resized_image = image.resize(size,size,1,3);
	
		    if(resized_image.spectrum() != 1){
			    CImg<double> gray = resized_image.get_RGBtoHSI().get_channel(2);
			    descriptors = Sift::compute_sift(gray);
                }
            
            //cout<<"image "<<hist+1<<endl;
            
            vector<vector<float> > descriptors_v = SiftDescriptor_to_vector(descriptors);
            //cout<<"number of descriptors "<<descriptors.size()<<endl;
            
            bow_histograms_vector.push_back(bow_histogram_calculation(centers_v, descriptors_v, clusterCount));
            
            //for(int v=0; v<clusterCount; v++)
            //    cout<<bow_histograms_vector[hist][v]<<" ";
            //cout<<endl;
            
            int sum_of_el = 0;
            for(std::vector<int>::iterator it = bow_histograms_vector[hist].begin(); it != bow_histograms_vector[hist].end(); ++it)
                sum_of_el += *it;      
            //cout<<"sum of elements in vector "<<sum_of_el<<endl;
       
            hist++;
            
        }
    }
    
    return bow_histograms_vector;
    
}


class BOW_SVM : public Classifier
{
public:
  BOW_SVM(const vector<string> &_class_list) : Classifier(_class_list) {
	  initMap();
	  inittarget_image_map();
  }
  

  virtual void train(const Dataset &filenames)
  {   
	system("./svm_multiclass_learn -c 1.0 bow_train_file.dat bow_svm_model");
  }
  
  

  virtual string classify(const string &filename){
	
    CImg<double> test_image(filename.c_str());    
                
    CImg<double> resized_image = test_image.resize(size,size,1,3);
    
	vector<SiftDescriptor> descriptors;
	
	if(resized_image.spectrum() != 1){
	    CImg<double> gray = resized_image.get_RGBtoHSI().get_channel(2);
	    descriptors = Sift::compute_sift(gray);
        }
    
    vector<vector<float> > descriptors_v = SiftDescriptor_to_vector(descriptors);
    
    Mat centers; 
    FileStorage fs("centers_40.yml", FileStorage::READ);
    fs["clusters"] >> centers;
    fs.release();
    
    vector<vector<float> > centers_v = Mat_to_vector(centers);

    vector<int> bow_histogram = bow_histogram_calculation(centers_v, descriptors_v, clusterCount);
	
	int size = bow_histogram.size();
    
	ofstream testFile;
	testFile.open("bow_test_file.dat");
    
	int feature_number = 1;  
	testFile<<1<<" ";
    
	for(int i=0; i<size - 1; i++){
		testFile<<feature_number<<":"<<bow_histogram[i]<<" ";
		feature_number++;
	}
    testFile<<feature_number<<":"<<bow_histogram[size - 1]<<endl;

	testFile.close();
	
	system(("./svm_multiclass_classify bow_test_file.dat bow_svm_model bow_predictions.dat"));
		
	int predicted_value = 1;
	ifstream myfile ("bow_predictions.dat");
	string line;
	myfile >> predicted_value;				
	cout<<"predicted value "<<predicted_value<<endl;
		
	string result = target_image_map[predicted_value];
	cout<<"result value "<<result<<endl;
    
	return result;
  }
		

  virtual void load_model()
	{
     }
public:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
	
    void initMap(){
		int value =1;
		for(int i =0;i<class_list.size();i++){
			image_target_map[class_list[i]] = value;
			value++;
	    }
    }
	
	void inittarget_image_map(){
		int value =1;
		for(int i =0;i<class_list.size();i++){
			target_image_map[value] = class_list[i];
			value++;
	    }
    }
	

    map<string, int> image_target_map;
    map<int, string> target_image_map;
 	
    static const int size=40;  // subsampled image resolution
    static const int clusterCount=500; //size of kmeans cluster in bow model
    map<string, CImg<double> > models; // trained models
    
    //vector<vector<int> > bow_histograms_vector;
};


//filenames = folder filenames, bow_histograms_vector = histogram vectors calculated using bow model
//takes folder name and histogram vectors and trains the histograms. then saves the trained model in "bow_train_file.dat" file
void bow_svm_train(const Dataset &filenames, vector<vector<int> > bow_histograms_vector, BOW_SVM *bow_svm){
   
    ofstream outputFile;
	outputFile.open("bow_train_file.dat");
    
    int histogram_size = bow_histograms_vector[0].size();
    
    int hist = 0;

    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter){
        
	    cout << "Processing " << c_iter->first <<endl;
	
        int target_class = bow_svm->image_target_map[c_iter->first];
        
	    cout<<target_class<<" ";

	    for(int i=0; i<c_iter->second.size(); i++){
            
            int feature_number = 1;

		    outputFile<<target_class<<" ";
            
		    for(int i=0; i<histogram_size - 1; i++){
			    outputFile<<feature_number<<":"<<bow_histograms_vector[hist][i]<<" ";
			    feature_number++;
		    }
            outputFile<<feature_number<<":"<<bow_histograms_vector[hist][histogram_size - 1]<<endl;
		    //feature_number = 1;
		} 
    }
     
    outputFile.close();
    
}






int main(int argc, char **argv)
{
  try {
    if(argc < 3)
      throw string("Insufficent number of arguments");

    string mode = argv[1];
    string algo = argv[2];

    // Scan through the "train" or "test" directory (depending on the
    //  mode) and builds a data structure of the image filenames for each class.
    Dataset filenames; 
    vector<string> class_list = files_in_directory(mode);
    for(vector<string>::const_iterator c = class_list.begin(); c != class_list.end(); ++c)
      filenames[*c] = files_in_directory(mode + "/" + *c, true);

    // Dataset filenames_test; 
    // vector<string> class_list_test = files_in_directory(algo);
    // for(vector<string>::const_iterator c = class_list_test.begin(); c != class_list_test.end(); ++c)
    //   filenames[*c] = files_in_directory(algo + "/" + *c, true);      
      
    //kmeans_clusters_centers(filenames, 40, 500);
    
    
    BOW_SVM *bow_svm=0; 
    if(algo == "bow")
    bow_svm = new BOW_SVM(class_list);
  
    if(mode == "train"){
       vector<vector<int> > bow_histograms_vector = bow_histograms_for_all_images_in_folder(filenames, 40, 500); 
       bow_svm_train(filenames, bow_histograms_vector, bow_svm);
       bow_svm->train(filenames);
    }

    else
       bow_svm->test(filenames);

/*
    // set up the classifier based on the requested algo
    Classifier *classifier=0;
    if(algo == "nn")
      classifier = new NearestNeighbor(class_list);
    else if(algo == "svm")
      classifier = new SVM(class_list);
    else
      throw std::string("unknown classifier " + algo);

    // now train or test!
    if(mode == "train")
      classifier->train(filenames);
    else if(mode == "test")
      classifier->test(filenames);
    else
      throw std::string("unknown mode!");
*/

  }

  catch(const string &err) {
      cerr << "Error: " << err << endl;
      }
  
}








