#include "CImg.h"

struct pattern_har_like{
		string pattern;
		int row;
		int col;
};
	
//this class only conatins the functionality requires by q2.2 (traditional features)
class svm_har{
private:
	vector<CImg<double> > har_like_patterns;	
	vector<pattern_har_like> seed_har_like_paterns;

public:
    
	CImg<double> extract_features(const string &filename){
		return extract_features(CImg<double>(filename.c_str()));
	}
	
	CImg<double> extract_features(const CImg<double>& in_image){
		CImg<double> r_image = in_image;
		r_image = r_image.resize(size,size);
		CImg<double> gray_scale_image = convert_to_gray_scale(r_image);
			gray_scale_image = zero_norm(gray_scale_image);
		CImg<double> intgrl_image = apply_integration(gray_scale_image);//follow like viola - jones
		CImg<double> img_har_feature = compute_har_feature(intgrl_image);
		return img_har_feature.unroll('x');
	}
	
	//call this method before calling any of the extract feature method for optimization reason
	void init(){
		load_seed_har();
		construct_har_pattern();		
	}
private:

    CImg<double> compute_har_feature(const CImg<double>& in_image){
		int w = in_image.width();
		int h = in_image.height();
		
		int pat_size = har_like_patterns.size();
		//cout<<"compute_har_feature::start::"<<pat_size<<endl;
		CImg<double> img_har_feature(pat_size*w*h,1);//pat_size*size*size
		//cout<<"feature size::"<<img_har_feature.size();
		int r_cnt = 0;
		for(int k = 0;k<pat_size;k++){
			CImg<double> pat_obj = har_like_patterns[k];
			int p_w = pat_obj.width();
			int p_h = pat_obj.height();
			for(int im_w=0;im_w<w;im_w++){
				for(int im_h=0;im_h<h;im_h++){
					img_har_feature(r_cnt,0) = 0;
			
					for(int x_w=0;x_w<p_w;x_w++){
						for(int x_h=0;x_h<p_h;x_h++){
							int o_w = x_w + im_w;
							int o_h = x_h + im_h;
							if(o_w >= w || o_h>=h){
								//we should wrap around to use other pixel value
								continue;
							}
							
							img_har_feature(r_cnt,0) = img_har_feature(r_cnt,0) + pat_obj(x_w,x_h)*in_image(o_w,o_h);
						}
					}//end of 2 for
					r_cnt++;
					////cout<<r_cnt<<endl;
				}
			}//end of 2 for
		}
		//cout<<"done"<<endl;
		return img_har_feature;
	}
	
    CImg<double> apply_integration(const CImg<double>& in_image){
		int w = in_image.width();
		int h = in_image.height();
		
		CImg<double> intgrl_img(w,h);
	
		//copy first row and first column
		for(int i=1;i<w;i++){
			intgrl_img(i,0) = in_image(i-1,0) + in_image(i,0);
		}
		
		for(int i=1;i<h;i++){
			intgrl_img(0,i) = in_image(0,i-1) + in_image(0,i);
		}
		
		for(int i=1;i<w;i++){
			for(int j=1;j<h;j++){
				double temp=in_image(i,j);
				 temp = temp + intgrl_img(i,j-1); 
				 temp = temp + intgrl_img(i-1,j);
				 temp = temp - intgrl_img(i-1,j-1);
				 intgrl_img(i,j)=temp;
			}
		}
		//cout<<"apply_integration"<<endl;
		return intgrl_img;
	}
	
	CImg<double> convert_to_gray_scale(const CImg<double>& in_image){
		int w = in_image.width();
		int h = in_image.height();
		
		CImg<double> o_image(w,h);		
		
		static float RED_F = 0.299;
		static float GREEN_F = 0.587;
		static float BLUE_F = 0.114;
				 
		for(int i=0;i<w;i++){
			for(int j=0;j<h;j++){
				o_image(i,j)=(RED_F*in_image(i,j,0,0))+(GREEN_F*in_image(i,j,0,1))+(BLUE_F*in_image(i,j,0,2));	
			}
		}
		//cout<<"g_s"<<endl;
		return o_image;
	}
	

	CImg<double> zero_norm(const CImg<double>& in_image){
		int w = in_image.width();
		int h = in_image.height();
		
		CImg<double> o_znormed_img(w,h);
		double mean  = in_image.mean();
		double std_dev = sqrt(in_image.variance());
	
		for(int i=0;i<w;i++){
			for(int j=0;j<h;j++){
				o_znormed_img(i,j) = (in_image(i,j) - mean)/std_dev;				
			}
		}	
		//cout<<"zero_norm"<<endl;
		return o_znormed_img;
	}
	
	void construct_har_pattern(){
		if(har_like_patterns.size()!=0){
			return;
		}
		int s_seed_size = seed_har_like_paterns.size();
		
		for(int s_i=0;s_i<s_seed_size;s_i++){
			pattern_har_like pat_det = seed_har_like_paterns[s_i];
			
			string pat_t = pat_det.pattern;
			int n_r = pat_det.row;
			int n_c = pat_det.col;
			
			CImg<double> pat_img(har_pat_size, har_pat_size);
			int o_r = 0;
			int o_c = 0;
			int pat_w_reg = pat_img.width()/n_c;
			int pat_h_reg = pat_img.height()/n_r;
			
			for(auto std::string::iterator b_w = pat_t.begin();b_w!=pat_t.end();b_w++){
				for(int i=0;i<pat_w_reg;i++){
					for(int j=0;j<pat_h_reg;j++){
						pat_img(i+o_c,j+o_r) = (*b_w == 'w')?1:-1;
					}
				}
				o_c = o_c + pat_w_reg;
				if(o_c>pat_img.width()){
					o_r = o_r + pat_h_reg;
					o_c = 0;
				}
			}
			//pat_img.save(("patern_harley_" + int_to_string(s_i) + ".png").c_str());
			har_like_patterns.push_back(pat_img);			
		}		
	}
	void load_seed_har(){
		if(seed_har_like_paterns.size()!=0){
			return;
		}
		//we can generate them randomly also, but that should be preserved for both train and test
		seed_har_like_paterns.push_back(make_har_pattern("wbbw",2,2));// 
		seed_har_like_paterns.push_back(make_har_pattern("wbw",3,1));
		seed_har_like_paterns.push_back(make_har_pattern("wb",2,1));
		seed_har_like_paterns.push_back(make_har_pattern("wbw",1,3));
		seed_har_like_paterns.push_back(make_har_pattern("wb",1,2));
		seed_har_like_paterns.push_back(make_har_pattern("wwbb",2,2));
		seed_har_like_paterns.push_back(make_har_pattern("bbww",2,2));
		seed_har_like_paterns.push_back(make_har_pattern("wbwb",2,2));
		seed_har_like_paterns.push_back(make_har_pattern("bwwb",2,2));
		seed_har_like_paterns.push_back(make_har_pattern("bwb",1,3));
		seed_har_like_paterns.push_back(make_har_pattern("bwb",3,1));
		seed_har_like_paterns.push_back(make_har_pattern("wwb",1,3));
		seed_har_like_paterns.push_back(make_har_pattern("wwb",3,1));
		seed_har_like_paterns.push_back(make_har_pattern("bw",1,2));
		seed_har_like_paterns.push_back(make_har_pattern("bw",2,1));	
	}
	
	pattern_har_like make_har_pattern(string p,int n_r,int n_c){
		pattern_har_like pt = {p,n_r,n_c};
		return pt;
	}
	
	string int_to_string(int i) {
		std::ostringstream out;
		out<<i;
		return out.str();
	}
	
	//global to all the instance
	static const int size = 40;
	static const int har_pat_size = 12;// multiple of 2,3 as used above in load_seed_har
};