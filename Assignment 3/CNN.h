#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class CNN : public Classifier
{
public:
  CNN(const vector<string> &_class_list) : Classifier(_class_list) {
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
		vector<double> myVector = extract_features(c_iter->second[i].c_str());
		int size = image_feature_vector.size();
		outputFile<<target_class<<" ";
		for(std::vector<int>::size_type i = 0; i != myVector.size()-1; i++) {
			outputFile<<feature_number<<":"<<myVector[i]<<" ";
			feature_number++;
		}
		outputFile<<feature_number<<":"<<myVector[myVector.size() -1]<<endl;

		
		feature_number = 1;
		} 
     }
    outputFile.close();
	system("./svm_multiclass_learn -c 1.0 train_file.dat svm_model");
  }
  
  

  virtual string classify(const string &filename){
    
		vector<double> myVector = extract_features(filename);
	
	//	int size = test_image.size();
		ofstream testFile;
		testFile.open("test_file.dat");
		int feature_number = 1;
		testFile<<1<<" ";
		
		for(std::vector<int>::size_type i = 0; i != myVector.size()-1; i++) {
			testFile<<feature_number<<":"<<myVector[i]<<" ";
			feature_number++;
		}
		
		testFile<<feature_number<<":"<<myVector[myVector.size() -1]<<endl;
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
  vector<double> extract_features(const string &filename)
    {
	
	CImg<double> image = CImg<double>(filename.c_str()).resize(231,231,1,3);
	
//	CImg<double> image = CImg<double>(filename.c_str());
	image.save_png("temp_image.png");
	

	ofstream myfile;
	system("overfeat/bin/linux_64/overfeat -f temp_image.png > features.dat");
	
	ifstream inFile ("features.dat");
	string nextToken;
	vector<double> myVector;
	
	string line;
   
    for (int i=0; i<2;i++)
    {
        getline(inFile,line);
    }
		
		istringstream iss(line);
		string part;
		while (getline(iss, part, ' '))
		{
	//	cout << "Token: " << part << endl;	
		myVector.push_back(atof(part.c_str()));	
		}
		return myVector;
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
 	
  static const int size=231;  // subsampled image resolution
  map<string, CImg<double> > models; // trained models
};