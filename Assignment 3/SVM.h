#include <iostream>
#include <fstream>
#include <string>

class SVM : public Classifier
{
public:
  SVM(const vector<string> &_class_list) : Classifier(_class_list) {
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
	CImg<double> class_vectors(size*size*3, filenames.size(), 1);
	
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
	system("./svm_multiclass_learn -c 1.0 train_file.dat svm_model");
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
	
		system("./svm_multiclass_classify test_file.dat svm_model predictions.dat");
		
		int predicted_value = 1;
		ifstream myfile ("predictions.dat");
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
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  CImg<double> extract_features(const string &filename)
    {
		CImg<double> image = CImg<double>(filename.c_str());
		
	//	CImg<double> bw_image = image.get_RGBtoYCbCr().get_channel(0);
	
      return image.resize(size,size,1,3).unroll('x');
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
	

  map<string, int> image_target_map;
  map<int, string> target_image_map;
 	
  static const int size=40;  // subsampled image resolution
  map<string, CImg<double> > models; // trained models
};
