#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "svm_har.h"

enum SVM_feature_extraction{svm_normal=1, svm_har_like=2, svm_pca=3};

class SVM_2 : public Classifier
{
public:
  SVM_2(const vector<string> &_class_list,const std::string& s_model_name, const unsigned int& svm_feature_extract) : Classifier(_class_list) {
	  svm_model_name = string(s_model_name);
	  category_feature_extract = svm_feature_extract;
	  
	  if(svm_feature_extract==svm_har_like){
		//hold the refrence for optimization reason
		svm_har_ref = new svm_har();
		svm_har_ref->init();  
	  }
	  
	  initMap();
	  inittarget_image_map();
  }
  

  virtual void train(const Dataset &filenames) 
  {
	ofstream outputFile;
	outputFile.open("train_file.dat");

    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
      {
	cout << "Processing " << c_iter->first <<endl;
	int target_class = image_target_map[c_iter->first];
	cout<<target_class<<endl;
	
	// convert each image to be a row of this "model" image
	CImg<double> image_feature_vector;

	int feature_number = 1;

	for(int i=0; i<c_iter->second.size(); i++){
		image_feature_vector = extract_features(c_iter->second[i].c_str());
		int size = image_feature_vector.size();
		outputFile<<target_class<<" ";
		for(int i=0;i<size - 1;i++){
			outputFile<<feature_number<<":"<<image_feature_vector(i,0)<<" ";
			feature_number++;
		}
		outputFile<<feature_number<<":"<<image_feature_vector(size -1,0)<<endl;
		feature_number = 1;
		} 
     }
    outputFile.close();
	
	switch(this->category_feature_extract){
			default:
			case svm_normal:
				system("./svm_multiclass_learn -c 1.0 train_file.dat svm_model_1");
				break;
			case svm_har_like:
				system("./svm_multiclass_learn  -c 0.1 -e 0.5 train_file.dat svm_model_2");
			break;
         	case svm_pca:
				system("./svm_multiclass_learn  -c 0.01 -e 0.5 train_file.dat svm_model_3");
			break;
	}
  }
  
  

  virtual string classify(const string &filename){
    CImg<double> test_image = extract_features(filename);
	
		int size = test_image.size();
		ofstream testFile;
		testFile.open("test_file.dat");
		int feature_number = 1;
		testFile<<1<<" ";
		for(int i=0;i<size - 1;i++){
			testFile<<feature_number<<":"<<test_image(i,0)<<" ";
			feature_number++;
		}
		testFile<<feature_number<<":"<<test_image(size -1,0)<<endl;
		testFile.close();
	
		system(("./svm_multiclass_classify test_file.dat "+this->svm_model_name+" predictions.dat").c_str());
		
		int predicted_value = 1;
		ifstream myfile ("predictions.dat");
		string line;
		myfile >> predicted_value;				
		cout<<"predicted value "<<predicted_value<<endl;
		
		string result = target_image_map[predicted_value];
		cout<<"result value "<<result<<endl;
		return result;
  }
		

  virtual void load_model(){
  }
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  CImg<double> extract_features(const string &filename){
		CImg<double> image = CImg<double>(filename.c_str());
		
	//	CImg<double> bw_image = image.get_RGBtoYCbCr().get_channel(0);
		switch(this->category_feature_extract){
			default:
			case svm_normal:			
				//comment the below line for gray scale images
				image = image.resize(size,size,1,3).unroll('x');
				
				//uncomment the following for gray scale image
				//image = image.resize(size,size,1,3).get_RGBtoYCbCr().get_channel(0).unroll('x');
				break;
			case svm_har_like:
			    image =  svm_har_ref->extract_features(image);
			break;  
			
		}	
      return image;
   }
		
	
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
	
	std::string strcat_multiple(const std::string sep,const vector<std::string>& strs){
		std::string s;
		int size = strs.size();
		for(int i=0;i<size;i++){
			if(i==size-1){
				s.append(strs[i]);
			}else{
				s.append(strs[i]+sep);
			}
		}
		return s;
	}
	
	const char * get_train_command(){
		string cmnd = "";
		switch(this->category_feature_extract){
			default:
			case svm_normal:
				cmnd = ("./svm_multiclass_learn -c 1.0 train_file.dat svm_model_1");
				cout<<cmnd;
				break;
			case svm_har_like:
				cmnd = ("./svm_multiclass_learn  -c "+int_to_string(C)+" -e 0.5 train_file.dat svm_model_2");
			break;
         	case svm_pca:
				cmnd = ("./svm_multiclass_learn  -c "+int_to_string(C)+" -e 0.5 train_file.dat svm_model_3");
			break;
		}
		return cmnd.c_str();
	}

	string int_to_string(int i) {
		std::ostringstream out;
		out<<i;
		return out.str();
	}
	
  //class varaible
  map<string, int> image_target_map;
  map<int, string> target_image_map;
  std::string svm_model_name;
  //this is related to assignment
  int category_feature_extract;
  svm_har* svm_har_ref = NULL;
  
  static const int size=40;  // subsampled image resolution
  static const int C = 50;
 };
