#include "cmath"
#include "algorithm"
#include "iostream"
#include "fstream"
#include "vector"
#include "SImage.h"
#include "DrawText.h"
#include "SImageIO.h"
//#include "utility"
#include "float.h"
using namespace std;

// The simple image class is called SDoublePlane, with each pixel represented as
// a double (floating point) type. This means that an SDoublePlane can represent
// values outside the range 0-255, and thus can represent squared gradient magnitudes,
// harris corner scores, etc.
//
// The SImageIO class supports reading and writing PNG files. It will read in
// a color PNG file, convert it to grayscale, and then return it to you in
// an SDoublePlane. The values in this SDoublePlane will be in the range [0,255].
//
// To write out an image, call write_png_file(). It takes three separate planes,
// one for each primary color (red, green, blue). To write a grayscale image,
// just pass the same SDoublePlane for all 3 planes. In order to get sensible
// results, the values in the SDoublePlane should be in the range [0,255].
//

// Below is a helper functions that overlays rectangles
// on an image plane for visualization purpose.


// DetectedSymbol class may be helpful!
//  Feel free to modify.
//
typedef enum {NOTEHEAD=0, QUARTERREST=1, EIGHTHREST=2} Type;
class DetectedSymbol {
public:
	int row, col;
	int width, height;
	Type type;
	char pitch;
	double confidence;

	DetectedSymbol();
	DetectedSymbol(int r, int c, int h, int w, Type t);
};

DetectedSymbol::DetectedSymbol(int r, int c, int h, int w, Type t){
	row = r;
	col = c;
	width = w;
	height = h;
	pitch = '\0';
	confidence = 0;
	type = (Type) (t);
	confidence = 100;
}



// Draws a rectangle on an image plane, using the specified gray level value and line width.
//
void overlay_rectangle(SDoublePlane &input, int _top, int _left, int _bottom, int _right, double graylevel, int width)
{
	for(int w=-width/2; w<=width/2; w++)
	{
		int top = _top+w, left = _left+w, right=_right+w, bottom=_bottom+w;

        // if any of the coordinates are out-of-bounds, truncate them
		top = min( max( top, 0 ), input.rows()-1);
		bottom = min( max( bottom, 0 ), input.rows()-1);
		left = min( max( left, 0 ), input.cols()-1);
		right = min( max( right, 0 ), input.cols()-1);

        // draw top and bottom lines
		for(int j=left; j<=right; j++)
			input[top][j] = input[bottom][j] = graylevel;
        // draw left and right lines
		for(int i=top; i<=bottom; i++)
			input[i][left] = input[i][right] = graylevel;
	}
}

// Function that outputs the ascii detection output file
void  write_detection_txt(const string &filename, const vector<struct DetectedSymbol> &symbols)
{
	ofstream ofs(filename.c_str());

	for(int i=0; i<symbols.size(); i++)
	{
		const DetectedSymbol &s = symbols[i];
		ofs << s.row << " " << s.col << " " << s.width << " " << s.height << " ";
		if(s.type == NOTEHEAD)
			ofs << "filled_note " << s.pitch;
		else if(s.type == EIGHTHREST)
			ofs << "eighth_rest _";
		else
			ofs << "quarter_rest _";
		ofs << " " << s.confidence << endl;
	}
}

// Function that outputs a visualization of detected symbols
void  write_detection_image(const string &filename, const vector<DetectedSymbol> &symbols, const SDoublePlane &input)
{
	SDoublePlane output_planes[3];
	for(int i=0; i<3; i++)
		output_planes[i] = input;

	for(int i=0; i<symbols.size(); i++)
	{
		const DetectedSymbol &s = symbols[i];

		overlay_rectangle(output_planes[s.type], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 255, 2);
		overlay_rectangle(output_planes[(s.type+1) % 3], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 0, 2);
		overlay_rectangle(output_planes[(s.type+2) % 3], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 0, 2);

		if(s.type == NOTEHEAD)
		{
			char str[] = {s.pitch, 0};
			draw_text(output_planes[0], str, s.row, s.col+s.width+1, 0, 2);
			draw_text(output_planes[1], str, s.row, s.col+s.width+1, 0, 2);
			draw_text(output_planes[2], str, s.row, s.col+s.width+1, 0, 2);
		}
	}

	SImageIO::write_png_file(filename.c_str(), output_planes[0], output_planes[1], output_planes[2]);
}

// refelect image to use for border processing by reflection
int reflect(int dim, int x)
{
    if(x < 0)
        return -x - 1;
    if(x >= dim)
        return 2*dim - x - 1;
    return x;
}
void normalize(SDoublePlane &img)
{

	double max = 0;
		// Calculate distance and max value
	for(int i = 0 ; i < img.rows(); ++i){
		for(int j = 0 ; j < img.cols() ; ++j){
			max = std::max(max, img[i][j]);
		}
	}

	// normalize img and find template location
	for(int k = 0 ; k < img.rows(); ++k){
		for(int l = 0 ; l < img.cols() ; ++l){
			img[k][l] = (img[k][l] / max) * 255.0;
		}
	}
}
void flipImageVertically(SDoublePlane &gx){

	for (int x = 0; x < (gx.rows()/2); x++)
	{
		for (int y = 0; y != gx.cols(); ++y)
		{
			std::swap(gx[x][y], gx[gx.rows() - 1 - x][y]);
		}
	}
}
// Convolve an image with a kxk convolution kernel
//
SDoublePlane convolve_general(const SDoublePlane &input, const SDoublePlane &filter)
{
	SDoublePlane output(input.rows(), input.cols());
	int s = filter.rows()/2;
	int cols = input.cols();
	int rows = input.rows();

	for(int i = s; i < rows - s; ++i){
		for(int j = s; j < cols - s; ++j){

			float sum = 0.0;

			for(int k = -s; k <= s; ++k) {
				for (int l = -s; l <= s; ++l) {

					sum += input[i+k][j+l] * filter[s-k][s-l];
				}
			}
			output[i][j] = sum;
		}
	}

	return output;
}


// The rest of these functions are incomplete. These are just suggestions to
// get you started -- feel free to add extra functions, change function
// parameters, etc.

// Convolve an image with a separable convolution kernel
//
SDoublePlane convolve_separable(const SDoublePlane &input, const SDoublePlane &row_filter, const SDoublePlane &col_filter)
{
    SDoublePlane output(input.rows(), input.cols());
    SDoublePlane temp(input.rows(), input.cols());
    //output = input;
    // Convolution code here
    int s = row_filter.rows()/2;
    //int s2 = col_filter.cols()/2;
    int cols = input.cols();
    int rows = input.rows();

    float sum;

        // row convolve
     for(int y = 0; y < rows; y++){
         for(int x = 0; x < cols; x++){
             sum = 0.0;
             for(int i = -s; i <= s; i++){
                 sum = sum + row_filter[s + i][0] * input[reflect(rows, y - i)][x];
             }
             temp[y][x] = sum;
            }
        }

      // colomn convolve
     for(int y = 0; y < rows; y++){
         for(int x = 0; x < cols; x++){
             sum = 0.0;
             for(int i = -s; i <= s; i++){
                 sum = sum + col_filter[0][s + i] * temp[y][reflect(cols, x - i)];
             }
              output[y][x] = sum;
             }
        }

    normalize(output);
    return output;
}

SDoublePlane cross_correlation(SDoublePlane &input, SDoublePlane &filter)
{
	SDoublePlane output(input.rows(), input.cols());
	int s = filter.rows()/2;
	int cols = input.cols();
	int rows = input.rows();

	for(int i = 0; i < rows; ++i){
		for(int j = 0; j < cols; ++j){

			float sum = 0.0;

			for(int k = -s; k <= s; ++k) {
				for (int l = -s; l <= s; ++l) {

					sum += input[reflect(rows, i+k)][reflect(cols, j+l)] * filter[s+k][s+l];
				}
			}
			output[i][j] = sum;
		}
	}

	normalize(output);
	return output;
}


void borderCorrector(const SDoublePlane &input,  SDoublePlane &output, int filterRow, int filterCol)
{

	int r = filterRow/2;
	int c = filterCol/2;
	int cols = input.cols();
	int rows = input.rows();

	for(int j = c ; j >= 0 ; --j){
		for(int i = 0 ; i < rows ; ++i){

			output[i][j] = (input[i][j] + output[i][j+1])/2;
		}
	}

	for(int j = (cols - c - 1) ; j < cols ; ++j){
		for(int i = 0 ; i < rows ; ++i){

			output[i][j] = (input[i][j] + output[i][j-1])/2;
		}
	}

	for(int i = r ; i >= 0 ; --i){
		for(int j = 0 ; j < cols ; ++j){
			output[i][j] = (input[i][j] + output[i+1][j])/2;
		}
	}

	for(int i = (rows - r - 1) ; i < rows ; ++i){
		for(int j = 0 ; j < cols ; ++j){
			output[i][j] = (input[i][j] + output[i-1][j])/2;
		}
	}
}


SDoublePlane binaryImage(const SDoublePlane &input, int inverted)
{
	SDoublePlane output(input.rows(), input.cols());
	for(int i = 0; i < input.rows() ; ++i)
	{
		for (int j = 0 ; j < input.cols(); ++j)
		{
			if(!inverted)
				output[i][j] = ((input[i][j] <= 128)? 0:1);
			else
				output[i][j] = ((input[i][j] <= 128)? 1:0);
		}
	}
	return output;
}


SDoublePlane makeMeanKernel(double size)
{
	SDoublePlane mean_filter(size,size);
	for(int i = 0; i < mean_filter.rows(); i++){
		for(int j = 0; j < mean_filter.cols(); j++){
			mean_filter[i][j] = 1/(size*size);
		}
	}

	return mean_filter;
}

SDoublePlane invertXYImage(const SDoublePlane &input)
{
	SDoublePlane output(input.rows(), input.cols());
	int row = input.rows();
	int col = input.cols();

	for(int i = 0; i < row ; ++i)
	{
		for (int j = 0 ; j < col ; ++j)
		{
			output[i][j] = input[row - 1 - i][col - 1 -j];
		}
	}
	return output;
}

/* the whole function makeGaussKernel1d was extracted from Book Principles of Digital Image Processing Fundamental Techniques, page 115. */
SDoublePlane makeGaussKernel1d(double sigma) {
// create the kernel
	int center = (int) (3.0*sigma);
  SDoublePlane kernel(1, 2*center+1);// odd size
  // fill the kernel
  double sigma2 = sigma * sigma; // σ 2
  for (int i = 0; i <= 2*center; i++) {
  	double r = center - i;
  	kernel[0][i] = (float) exp(-0.5 * (r*r) / sigma2);
  	// cout<< kernel[0][i]<<endl;
  }
  return kernel;
}



SDoublePlane sobel_hough_gradient_filter(const SDoublePlane &input, bool _gx)
{
    SDoublePlane output(input.rows(), input.cols());
	int cols = input.cols();
	int rows = input.rows();
    if (_gx){
        cout<<"inside"<<endl;
        double temp_sobel_filer_horizontal_hx[3][1]= {{1},{2},{1}};
        double temp_sobel_filer_horizontal_hy[1][3]= {{1,0,-1}};
        int x[3][3] = {{1,0,-1},{2,0,-2},{1,0,-1}};
        for(int i=1;i<input.rows()-1;i++){
            for(int j=1;j<input.cols()-1;j++){
                int sum=0;
                for(int m=-1; m<=1; m++){
                    for(int n=-1; n<=1; n++)
                    {
                        sum += input[i+m][j+n] * x[m+1][n+1];
                    }
                }
                if (sum>255.00)
                    sum=255;

                output[i][j]=sum;
            }
        }


    return output;
    }
    else if (! _gx){
        cout<<"inside2"<<endl;
        int y[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
        for(int i=1;i<input.rows()-1;i++){
            for(int j=1;j<input.cols()-1;j++){
                int sum=0;
                for(int m=-1; m<=1; m++){
                    for(int n=-1; n<=1; n++)
                    {
                        sum += input[i+m][j+n] * y[m+1][n+1];
                    }
                }
                if (sum>255.00)
                    sum=255;

                output[i][j]=sum;
            }
        }

    return output;

    }
	return output;
}


SDoublePlane convert_image_to_binary(const SDoublePlane &input)
{
	SDoublePlane output(input.rows(), input.cols());
	for(int i = 0; i < input.rows(); i++)
	{
		for (int j = 0; j < input.cols(); j++)
		{
			if (input[i][j] > 175)
				output[i][j] = 255;
			else
				output[i][j] = 0;
		}
	}
	return output;
}


vector<DetectedSymbol> detectTemplate(const SDoublePlane &input, const SDoublePlane &temp, Type type, double threshold = 245)
{

	SDoublePlane inputBin = binaryImage(input,0);
	SDoublePlane inputBinInv = binaryImage(input,1);

	SDoublePlane tempBin = binaryImage(temp,0);
	SDoublePlane tempBinInv = binaryImage(temp,1);

	tempBin = invertXYImage(tempBin);
	tempBinInv = invertXYImage(tempBinInv);

	SDoublePlane output = convolve_general(inputBin, tempBin);
	SDoublePlane outputInv = convolve_general(inputBinInv, tempBinInv);

	// borderCorrector(inputBin, output, tempBin.rows(), tempBin.cols());
	// borderCorrector(inputBinInv, outputInv, tempBinInv.rows(), tempBinInv.cols());

	normalize(output);
	normalize(outputInv);
	// SImageIO::write_png_file("convolve1.png", output, output, output);
	// SImageIO::write_png_file("convolve2.png", outputInv, outputInv, outputInv);

	SDoublePlane distance(output.rows(), output.cols());
	vector<DetectedSymbol> symbols;

	// Calculate distance between two images
	for(int i = 0 ; i < output.rows(); ++i)
	{
		for(int j = 0 ; j < output.cols() ; ++j)
		{
			distance[i][j] = output[i][j] + outputInv[i][j];
		}
	}

	normalize(distance);
	// borderCorrector(input, distance, temp.rows(), temp.cols());
	// SImageIO::write_png_file("distance.png", distance, distance, distance);


	// find template location and perform non maximum suppression
	for(int k = 0 ; k < distance.rows(); ++k)
	{
		for(int l = 0 ; l < distance.cols() ; ++l)
		{
			if (distance[k][l] > threshold)
			{
				bool flag = false;
				for (int i = -temp.rows()/2; i <= temp.rows()/2; ++i)
				{
					for (int j = -temp.cols()/2; j <= temp.cols()/2; ++j)
					{
						if(distance[k+i][l+j] > distance[k][l] && flag == false)
							flag = true;
					}
				}

				if (flag == false)
				{
					DetectedSymbol sym (k, l , temp.rows(), temp.cols(), (Type) type);
					symbols.push_back(sym);
				}
			}
		}
	}

	cout << "symbols found: " << symbols.size() << endl;
	return symbols;
}


void findPitch(vector<DetectedSymbol> &sym, const int &bottom, const int &top, const int &dist, bool treble)
{
	vector<char> pitchLetter;
	vector<int> pitchLoc;

	if (treble)
	{
		char letters[] = {'B', 'C', 'D', 'E', 'F', 'G', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'A', 'B'};
		pitchLetter.assign(letters, letters + sizeof(letters));

		pitchLoc.push_back(top + (5.5)*dist);
		pitchLoc.push_back(top + 5*dist);
		pitchLoc.push_back(top + (4.5)*dist);
		pitchLoc.push_back(top + 4*dist);
		pitchLoc.push_back(top + (3.5)*dist);
		pitchLoc.push_back(top + 3*dist);
		pitchLoc.push_back(top + (2.5)*dist);
		pitchLoc.push_back(top + 2*dist);
		pitchLoc.push_back(top + (1.5)*dist);
		pitchLoc.push_back(top + dist);
		pitchLoc.push_back(top + (0.5)*dist);
		pitchLoc.push_back(top);
		pitchLoc.push_back(top - (0.5)*dist);
		pitchLoc.push_back(top - dist);
		pitchLoc.push_back(top - (1.5)*dist);
	}
	else
	{

		char letters[] = {'D', 'E', 'F', 'G', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'A', 'B', 'C', 'D'};
		pitchLetter.assign(letters, letters + sizeof(letters));

		pitchLoc.push_back(top + (5.5)*dist);
		pitchLoc.push_back(top + 5*dist);
		pitchLoc.push_back(top + (4.5)*dist);
		pitchLoc.push_back(top + 4*dist);
		pitchLoc.push_back(top + (3.5)*dist);
		pitchLoc.push_back(top + 3*dist);
		pitchLoc.push_back(top + (2.5)*dist);
		pitchLoc.push_back(top + 2*dist);
		pitchLoc.push_back(top + (1.5)*dist);
		pitchLoc.push_back(top + dist);
		pitchLoc.push_back(top + (0.5)*dist);
		pitchLoc.push_back(top);
		pitchLoc.push_back(top - (0.5)*dist);
		pitchLoc.push_back(top - dist);
		pitchLoc.push_back(top - (1.5)*dist);
	}

	for (int i = sym.size() - 1; i >= 0 ; --i)
	{
		int real = sym.at(i).row;
		int j = 1;
		int ideal = pitchLoc.at(j);

		if (real >= (top - 2*dist) && real <= (bottom + 2*dist))
		{
			while((ideal+4) > real  && j < pitchLoc.size()){

				if(ideal == real){
					break;
				}

				if(j >= pitchLoc.size()-1){
					break;
				}

				++j;
				ideal = pitchLoc.at(j);
			}
			sym.at(i).pitch = pitchLetter.at(j);
		}

	}

}


bool detectBass(const SDoublePlane &input, const SDoublePlane &temp, int top, int bottom){

	SDoublePlane auxImg(temp.rows() + temp.rows()/2, input.cols());


	for (int i = 0; i < auxImg.rows(); ++i)
	{
		for (int j = 0; j < auxImg.cols(); ++j)
		{
			auxImg[i][j] = input[top+i][j];
		}
	}

	// SImageIO::write_png_file("auxImg.png", auxImg, auxImg, auxImg);

	vector<DetectedSymbol> bass = detectTemplate(auxImg, temp, (Type) 0, 254);
	if(bass.size() > 0){
		cout << bass.at(0).row << " " << bass.at(0).col << endl;
		return true;
	}
	else{
		return false;
	}

}
// Apply a sobel operator to an image, returns the result
//
SDoublePlane sobel_gradient_filter(const SDoublePlane &input, bool _gx)
{
	  // Implement a sobel gradient estimation filter with 1-d filters
/*
    //vertical component of sobel operator
    int vertical_col[] = {1, 2, 1};
    int vertical_row[] = {-1, 0, 1};
    //horizontal component of sobel operator
    int horizontal_col[] = {-1, 0, 1};
    int horizontal_row[] = {1, 2, 1}; */

    SDoublePlane sobel_hx_v(3,1);
    sobel_hx_v[0][0] = 1;
    sobel_hx_v[1][0] = 2;
    sobel_hx_v[2][0] = 1;

    SDoublePlane sobel_hx_h(1,3);
    sobel_hx_h[0][0] = -1;
    sobel_hx_h[0][1] = 0;
    sobel_hx_h[0][2] = 1;

    SDoublePlane sobel_hy_v(3,1);
    sobel_hy_v[0][0] = -1;
    sobel_hy_v[1][0] = 0;
    sobel_hy_v[2][0] = 1;

    SDoublePlane sobel_hy_h(1,3);
    sobel_hy_h[0][0] = 1;
    sobel_hy_h[0][1] = 2;
    sobel_hy_h[0][2] = 1;

    if(_gx)
    {
 	   SDoublePlane sobel_hx = convolve_separable(input, sobel_hx_v, sobel_hx_h);
        return sobel_hx;
    }
    else
    {
	   SDoublePlane sobel_hy = convolve_separable(input, sobel_hy_v, sobel_hy_h);
        return sobel_hy;
    }


}

// Apply an edge detector to an image, returns the binary edge map
//
SDoublePlane find_edges(const SDoublePlane &input, SDoublePlane sobel_hx,SDoublePlane sobel_hy,double thresh=0)
{
    SDoublePlane output(input.rows(), input.cols());

    int cols = input.cols();
    int rows = input.rows();

    //double gauss[] = {0.0545, 0.2442, 0.4026, 0.2442, 0.0545};

    //SDoublePlane sobel_hx = sobel_gradient_filter(input, true);
    //SImageIO::write_png_file("sobel_hx.png", sobel_hx, sobel_hx, sobel_hx);

    //SDoublePlane sobel_hy = sobel_gradient_filter(input, false);
    //SImageIO::write_png_file("sobel_hy.png", sobel_hy, sobel_hy, sobel_hy);

    // Implement an edge detector of your choice, e.g.
    // use your sobel gradient operator to compute the gradient magnitude and threshold

	for(int i = 0; i < rows; ++i){

		for(int j = 0; j < cols; ++j){

            output[i][j] = abs(sobel_hx[i][j]) + abs(sobel_hy[i][j]);

        }
    }

    normalize(output);
	return output;
}

vector<DetectedSymbol> templateMatchingThreshold_1(SDoublePlane &input, Type t)
{
    SDoublePlane output(input.rows(), input.cols());
    vector<DetectedSymbol> noteboxes;
    for(int i=0; i<input.rows(); i++)
    {
        for (int j=0; j<input.cols(); j++)
        {
            if (input[i][j] > 15){
                output[i][j]=255;

            }
            else{
                output[i][j] < 15;
                DetectedSymbol s1(i-5,j,10,10,t);
                noteboxes.push_back(s1);
        }
    }}

    return noteboxes;
}

vector<DetectedSymbol> templateMatchingThreshold_2(SDoublePlane &input, Type t)
{
    SDoublePlane output(input.rows(), input.cols());
    vector<DetectedSymbol> noteboxes;
    for(int i=0; i<input.rows(); i++)
    {
        for (int j=0; j<input.cols(); j++)
        {
            if (input[i][j] > 30){
                output[i][j]=255;

            }
            else{
                output[i][j] < 30;
                DetectedSymbol s1(i-5,j,15,15,t);
                noteboxes.push_back(s1);
        }
    }}

    return noteboxes;
}

vector<DetectedSymbol> templateMatchingThreshold_3(SDoublePlane &input, Type t)
{
    SDoublePlane output(input.rows(), input.cols());
    vector<DetectedSymbol> noteboxes;
    for(int i=0; i<input.rows(); i++)
    {
        for (int j=0; j<input.cols(); j++)
        {
            if (input[i][j] > 60){
                output[i][j]=255;

            }
            else{
                output[i][j] < 60;
                DetectedSymbol s1(i-5,j,18,18,t);
                noteboxes.push_back(s1);
        }
    }}

    return noteboxes;
}



SDoublePlane distanceTransform(SDoublePlane &input) {

	SDoublePlane output(input.rows(), input.cols());

	int rows = input.rows();
	int cols = input.cols();

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){

            float min_val = FLT_MAX;
            float dist;
            // if the point has value 1 we will give output = 0
            if(input[i][j] > 0){
                output[i][j] = 0;
            }
            else
            {
			    for(int a = 0; a < rows; a++){
				    for(int b = 0; b < cols; b++){

					    if(input[a][b] > 0){
					        dist = sqrt(pow(i-a,2) + pow(j-b,2));
                            if(dist < min_val)
							    min_val = dist;
                         }
				    }
			    }
			    output[i][j] = 255;

            }

		}
	}
    //normalize(output);
	return output;
}



SDoublePlane getFinalAccumulatorOutput(SDoublePlane &image){
	SDoublePlane threshold_acc(image.rows(),image.cols());
	SDoublePlane accumultor(image.rows(),image.cols());
	for(int x=0;x<image.rows();x++)
	{
		for(int y=0; y<image.cols(); y++)
		{
			if(175<=image[x][y])
			{
				for(int j=x+5; j<image.rows();j=j+1){
					if(175<=image[j][y])
						accumultor[x][j-x]++;

				}
			}

		}

	}
    //Thresholding on accumulator
	for(int x=0;x<image.rows();x++)
	{
		for(int y=0; y<image.cols(); y++)
		{
			if (image[x][y]>175 && accumultor[x][y]>400)
			{
				for(int j=0; j<image.cols(); j++)
				{
					threshold_acc[x+y][j]=255;
				}
			}

		}
	}
	return threshold_acc;
}


std::pair<int, vector<int> >  hough_transform(SDoublePlane &input_binary_image, SDoublePlane &gx, SDoublePlane &gy, SDoublePlane &final_image){

 //SDoublePlane final_image(gx.rows(),gx.cols());
 SDoublePlane new_gy(gy.rows(),gy.cols());
 SDoublePlane start_lines(gx.rows(),gx.cols());
 for(int x=0;x<gy.rows();x=x+1)
        for(int y=0; y<gy.cols(); y++)
            new_gy[x][y]=gy[x][y];

 int line_position[100];
 //int final_line_end_points[][]=new int[100][100];
 SDoublePlane ouput1=getFinalAccumulatorOutput(new_gy);

 flipImageVertically(new_gy);
//SImageIO::write_png_file("flipped.png", gy, gy, gy);
 SDoublePlane output2=getFinalAccumulatorOutput(new_gy);
    flipImageVertically(output2);
    bool previous_flag= false;
    int counter=0;
    vector<DetectedSymbol> houghBoxs;
    for(int x=0;x<gx.rows();x=x+1){
        if(previous_flag){
            previous_flag=false;
            continue;
        }
        for(int y=0; y<gx.cols(); y++)
        {
            if(output2[x][y]>200 || ouput1[x][y]>200){
                previous_flag=true;
                final_image[x][y]=255;
            }

        }
        if (previous_flag){
            //DetectedSymbol houghsymbol(x,0,4,input_binary_image.cols(),(Type)2);

            line_position[counter]=x;
            counter++;
            //cout<<x<<endl;
            }
    }

    int c=0;
    int ctr=0;
    int diffcount=0;
    vector<int> valid_point;
    valid_point.push_back(line_position[0]);
    c++;
    for(int i=1;i<counter;i++){
        if(5 < abs(line_position[i]-line_position[i-1]) && abs(line_position[i]-line_position[i-1])< 20)
        {
            diffcount+=abs(line_position[i]-line_position[i-1]);
//            cout<<line_position[i]<<" "<< line_position[i-1]<<endl;
            ctr++;
        }
        if(abs(line_position[i]-line_position[i-1]) > 50)
        {
            valid_point.push_back(line_position[i]);
            c++;
            //cout<<line_position[i]<<endl;
        }

    }
    //cout<<"Count"<<diffcount<<" "<< ctr<<endl;
    int avg_diff_count=diffcount/ctr;
    cout<<"distance: "<<avg_diff_count<<" "<< ctr<<endl;
    for(int x=0;x<valid_point.size();x++)
    {
        int p= valid_point[x];
        cout<<p<<endl;
        for(int y=0; y<5; y++)
        {
            DetectedSymbol houghsymbol(p+avg_diff_count*y,0,4,input_binary_image.cols(),(Type)2);
            houghBoxs.push_back(houghsymbol);
        }
    }
     write_detection_image("staves.png",houghBoxs, input_binary_image);
    std::pair<int, vector<int> > twoValues;
    twoValues.first=avg_diff_count;
    twoValues.second=valid_point;
    return twoValues;



}

SDoublePlane generate_edge_map(SDoublePlane &input)
{
	SDoublePlane output(input.rows(), input.cols());
	for(int i=0; i<input.rows(); i++)
	{
		for (int j=0; j<input.cols(); j++)
		{
			if (input[i][j] > 75)
				output[i][j]=1;
			else
				output[i][j]=0;
		}
	}
	return output;
}

SDoublePlane generate_edge_map_for_image(SDoublePlane &input)
{
    SDoublePlane output(input.rows(), input.cols());
    for(int i=0; i<input.rows(); i++)
    {
        for (int j=0; j<input.cols(); j++)
        {
            if (input[i][j] > 75)
                output[i][j]=200;
            else
                output[i][j]=0;
        }
    }
    
    return output;
}