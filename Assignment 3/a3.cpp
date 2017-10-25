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
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Sift.h>
#include <sys/types.h>
#include <dirent.h>
#include <map>
#include <numeric>
#include <fstream>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;

// Dataset data structure, set up so that e.g. dataset["bagel"][3] is
// filename of 4th bagel image in the dataset
typedef map<string, vector<string> > Dataset;

#include <Classifier.h>
#include <NearestNeighbor.h>
#include <SVM_2.h>
#include <CNN.h>

#define SVM_NORMAL_CATG "N"


//prototype of different functions
bool is_valid_mode(const std::string mode);
void process(const std::string mode,Classifier *classifier,const Dataset& filenames);
void process_pca_svm();
Classifier * get_classifier(const std::string algo, const std::string &mode, const vector<string> &_class_list);

int get_svm_feature_extr_cat(string cat);
std::string get_svm_model_name(const std::string &mode);
std::string get_svm_model_name(const int &svm_cat);

// Figure out a list of files in a given directory.
//
vector<string> files_in_directory(const string &directory, bool prepend_directory = false)
{
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

string g_svm_feature_catg = SVM_NORMAL_CATG;

void create_class_list_dump(vector<string> class_list){
		ofstream classListDump("dumpFile.dat");
	   int s = class_list.size();
	   for(int i=1;i<=s;i++){
		   classListDump<<class_list[i-1]<<" "<<i<<endl;
	   }
	   classListDump.close();
}

vector<string> read_class_list(){
	string s = "dumpFile.dat";
	vector<string> classList;
	ifstream readList(s.c_str());
	string line;
	while ( getline (readList,line) )
    {
		istringstream iss(line);
		string part;
      while (getline(iss,part , ' '))
		{
		classList.push_back(part);
		break;		
		}
    }
    readList.close();
		return classList;
}


int main(int argc, char **argv)
{
  try {
	
    if(argc < 3)
      throw std::string("Insufficent number of arguments");

	
    string mode = argv[1];
    string algo = argv[2];
	
	//ovveride if any
	g_svm_feature_catg = argc>3 ? argv[3]:"N";
	
	//validate mode
	if(!is_valid_mode(mode)){
		throw std::string("unknown mode!");
	}
	
	//pca with svm will run if
	if(algo =="svm" && g_svm_feature_catg=="P"){
		if(mode=="train"){
			//creating pca_train and pca_test, folders for svm+pca using python
			process_pca_svm();
			mode = "pca_train";
		}else if(mode=="test"){
			//change it to pca_test
			mode = "pca_test";
		}else {
			throw std::string("Invalid mode with P argument: command : ./a3 [train|test] svm P");
		}
	}
	
	// Scan through the "train" or "test" directory (depending on the
    //  mode) and builds a data structure of the image filenames for each class.
    Dataset filenames; 
    vector<string> class_list ;
	if(mode=="train" || mode=="pca_train"){
		class_list = files_in_directory(mode);
			//dump to a temp file
			create_class_list_dump(class_list);
			
	}else if(mode=="test" || mode=="pca_test"){
		//read from a file
		class_list = read_class_list();
	}
	
    for(vector<string>::const_iterator c = class_list.begin(); c != class_list.end(); ++c)
      filenames[*c] = files_in_directory(mode + "/" + *c, true);

    
    // set up the classifier based on the requested algo
    Classifier *classifier=get_classifier(algo, mode, class_list);
	
	if(NULL==classifier){
	   throw std::string("unknown classifier " + algo);
	}   

    if(mode=="pca_train"){
		mode = "train";
	}else if(mode=="pca_test"){
		mode = "test";
	}
	
	// now train or test!
	process(mode, classifier, filenames);       
  }catch(const string &err) {
    cerr << "Error: " << err << endl;
  } 
}

void process(const std::string mode, Classifier *classifier,const Dataset& filenames){
	if(mode == "train"){  
		classifier->train(filenames);
	}
	else if(mode == "test"){
		classifier->test(filenames);
    }
}

void process_pca_svm(){
	system("python3 pca_2_cv.py");
}

std::string get_svm_model_name(const std::string &mode){
	  string svm_model_name = "svm_model_";
	  if(mode=="train"){
		  svm_model_name = svm_model_name+"1";
	  }else if(mode=="train_2"){
		svm_model_name = svm_model_name+"2";
	  }
	  return svm_model_name;
}

std::string get_svm_model_name(const int &svm_cat){
	  string svm_model_name = "svm_model_";
	  if(svm_cat==svm_normal){
		  svm_model_name = svm_model_name+"1";
	  }else if(svm_cat==svm_har_like){
		svm_model_name = svm_model_name+"2";
	  }else if(svm_cat==svm_pca){
		svm_model_name = svm_model_name+"3";  
	  }
	  return svm_model_name;
}

Classifier * get_classifier(const std::string algo,const std::string &mode, const vector<string> &class_list){
	//cout<<algo;
	Classifier *classifier = NULL;
	 if(algo == "nn"){
       classifier = new NearestNeighbor(class_list);
	 }
	 if(algo == "cnn")
	 {
	   classifier = new CNN(class_list);	 
		 
	 }
	/* Part 1 code starts here */
	if(algo == "svm"){
		int svm_catg_type = get_svm_feature_extr_cat(g_svm_feature_catg);
		std::string svm_model_name = get_svm_model_name(svm_catg_type);
		classifier = new SVM_2(class_list, svm_model_name, svm_catg_type);
	}
	return classifier;	
}

int get_svm_feature_extr_cat(string cat){
		int cat_type = svm_normal;
		//cout<<cat;
		if(cat!="N" && cat!="" && cat!="H" && cat!="P"){
			throw std::string("Invalid SVM Command Argument. pass valid categories: H [for har-like], P[pca with svm], default[N], command: ./a3 train svm [H|P]");
		}
		if(cat=="H"){
			cat_type = svm_har_like;
		}
		if(cat=="P"){
			cat_type = svm_pca;
		}
		return cat_type;
}

bool is_valid_mode(const std::string mode){
	return mode == "train" || mode == "test";	
}