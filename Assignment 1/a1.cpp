 #include "a1_utils.h"
using namespace std;



void problem2(const SDoublePlane &input_image)
{


	SDoublePlane mean_filter = makeMeanKernel(3);

	SDoublePlane convolve = convolve_general(input_image, mean_filter);
	normalize(convolve);

	SImageIO::write_png_file("problem2.png", convolve, convolve, convolve);

	borderCorrector(input_image, convolve, mean_filter.rows(), mean_filter.cols());
	SImageIO::write_png_file("problem2_borders.png", convolve, convolve, convolve);

}

void problem3(const SDoublePlane &input_image)
{
	SDoublePlane x_gaussian_kernel = makeGaussKernel1d(0.5);
	SDoublePlane y_gaussian_kernel = makeGaussKernel1d(0.5);

	SDoublePlane convolve2 = convolve_separable(input_image, x_gaussian_kernel,y_gaussian_kernel);
	normalize(convolve2);

	SImageIO::write_png_file("problem3.png", convolve2, convolve2, convolve2);
	borderCorrector(input_image, convolve2, x_gaussian_kernel.cols(), y_gaussian_kernel.cols());
	SImageIO::write_png_file("problem3_borders.png", convolve2, convolve2, convolve2);
}

void problem4(const SDoublePlane &input_image, const vector<SDoublePlane> &templates, int imgNum)
{


	// vector<DetectedSymbol> symbols;
	// vector<DetectedSymbol> symbolsAux;
	// int threshold = 226;
	// for (int i = 0; i < templates.size(); ++i)
	// {
	// 	if(i == 0){
	// 		// cout << "yes NOTEHEAD " << i << endl;
	// 		symbolsAux = detectTemplate(input_image, templates.at(i), (Type)(i), threshold);
	// 	}
	// 	else{
	// 		// cout << "not NOTEHEAD " << i << endl;
	// 		symbolsAux = detectTemplate(input_image, templates.at(i), (Type)(i), 240);
	// 	}

	// 	symbols.insert(symbols.end(), symbolsAux.begin(), symbolsAux.end());
	// }


	// write_detection_txt("detected.txt", symbols);
	// write_detection_image("detected.png", symbols, input_image);


//////////////////////////////////// Hardcoded values of staff lines in each of the input images

/* music1.png */
	if (imgNum == 1)
	{
		vector<DetectedSymbol> symbols1 = detectTemplate(input_image, templates.at(0), (Type)(0), 228);
		int bottom = 198;
		int dist = 10;
		int top = bottom - 6 * dist;
		findPitch(symbols1, bottom, top, dist, false);
		bottom = 78;
		dist = 10;
		top = bottom - 6 * dist;
		findPitch(symbols1, bottom, top, dist, true);
		vector<DetectedSymbol> symbols2 = detectTemplate(input_image, templates.at(1), (Type)(1), 250);
		vector<DetectedSymbol> symbols3 = detectTemplate(input_image, templates.at(2), (Type)(2), 250);
		write_detection_txt("detected4.txt", symbols3);
		write_detection_image("detected4.png", symbols3, input_image);
	}

/* music2.png */
	if (imgNum == 2)
	{
		vector<DetectedSymbol> symbols1 = detectTemplate(input_image, templates.at(0), (Type)(0), 229);
		int bottom = 275;
		int dist = 13;
		int top = bottom - 6 * dist;
		findPitch(symbols1, bottom, top, dist, true);

		bottom = 130;
		top = bottom - 6 * dist;
		findPitch(symbols1, bottom, top, dist, true);
		vector<DetectedSymbol> symbols2 = detectTemplate(input_image, templates.at(1), (Type)(1), 250);
		vector<DetectedSymbol> symbols3 = detectTemplate(input_image, templates.at(2), (Type)(2), 240);
		write_detection_txt("detected4.txt", symbols3);
		write_detection_image("detected4.png", symbols3, input_image);
	}

/* music3.png */
	if (imgNum == 3)
	{
		vector<DetectedSymbol> symbols1 = detectTemplate(input_image, templates.at(0), (Type)(0), 228);
		int bottom = 92;
		int dist = 10;
		int top = bottom - 8 * dist;
		findPitch(symbols1, bottom, top, dist, true);

		bottom = 192;
		top = bottom - 8 * dist;
		findPitch(symbols1, bottom, top, dist, false);

		bottom = 345;
		top = bottom - 8 * dist;
		findPitch(symbols1, bottom, top, dist, true);

		bottom = 445;
		top = bottom - 8 * dist;
		findPitch(symbols1, bottom, top, dist, false);

		vector<DetectedSymbol> symbols2 = detectTemplate(input_image, templates.at(1), (Type)(1), 250);
		vector<DetectedSymbol> symbols3 = detectTemplate(input_image, templates.at(2), (Type)(2), 250);
		write_detection_txt("detected4.txt", symbols3);
		write_detection_image("detected4.png", symbols3, input_image);
	}
/* music4.png */
	if (imgNum == 4)
	{
		vector<DetectedSymbol> symbols1 = detectTemplate(input_image, templates.at(0), (Type)(0), 229);

		int bottom = 92;
		int dist = 10;
		int top = bottom - 8 * dist;
		findPitch(symbols1, bottom, top, dist, true);

		bottom = 192;
		top = bottom - 8 * dist;
		findPitch(symbols1, bottom, top, dist, false);

		vector<DetectedSymbol> symbols2 = detectTemplate(input_image, templates.at(1), (Type)(1), 250);
		vector<DetectedSymbol> symbols3 = detectTemplate(input_image, templates.at(2), (Type)(2), 240);

		// concat all founded symbols
		symbols2.insert(symbols2.end(), symbols1.begin(), symbols1.end());
		symbols3.insert(symbols3.end(), symbols2.begin(), symbols2.end());
		write_detection_txt("detected4.txt", symbols3);
		write_detection_image("detected4.png", symbols3, input_image);
	}


}


void problem5(SDoublePlane &input_image,vector<SDoublePlane> templates)
{

    SDoublePlane image_sobel_hx = sobel_gradient_filter(input_image, true);
    SDoublePlane image_sobel_hy = sobel_gradient_filter(input_image, false);

    SDoublePlane sobel_edge = find_edges(input_image,image_sobel_hx,image_sobel_hy);

    SDoublePlane sobel_output = generate_edge_map_for_image(sobel_edge);


    //SDoublePlane sobel_template11
    SDoublePlane distance_value = distanceTransform(sobel_output);

    //Store all the sobel templates in the below variable

    vector<DetectedSymbol> allthenoteboxes;
   /* for(int i=0; i<templates.size();i++){
        SDoublePlane temp= templates[i];
        SDoublePlane sobel_hx = sobel_gradient_filter(temp, true);
        SDoublePlane sobel_hy = sobel_gradient_filter(temp, false);
        SDoublePlane sobel_template=find_edges(temp,sobel_hx,sobel_hy);
        cout<<"Template Matching "<<i+1<< "  50% done"<<endl;
        SDoublePlane template_matching = cross_correlation(distance_value, sobel_template);
        vector<DetectedSymbol> notebox= templateMatchingThreshold(template_matching, (Type) i,i);
        cout<<"Template Matching "<<i+1<< " completly done"<<endl;
        allthenoteboxes.insert(allthenoteboxes.end(), notebox.begin(), notebox.end());
    }*/

    SDoublePlane temp1 = templates[0];
    SDoublePlane sobel_hx = sobel_gradient_filter(temp1, true);
    SDoublePlane sobel_hy = sobel_gradient_filter(temp1, false);
    SDoublePlane sobel_template1 = find_edges(temp1, sobel_hx, sobel_hy);

    SDoublePlane sobel_template11 = generate_edge_map(sobel_template1);

    SDoublePlane template_matching1 = cross_correlation(distance_value, sobel_template11);

    vector<DetectedSymbol> notebox1 = templateMatchingThreshold_1(template_matching1, (Type)0);
    allthenoteboxes.insert(allthenoteboxes.end(), notebox1.begin(), notebox1.end());
    write_detection_image("edge1.png", notebox1, input_image);


    SDoublePlane temp2 = templates[1];
    SDoublePlane sobel_hx2 = sobel_gradient_filter(temp2, true);
    SDoublePlane sobel_hy2 = sobel_gradient_filter(temp2, false);
    SDoublePlane sobel_template2 = find_edges(temp2, sobel_hx2, sobel_hy2);

    SDoublePlane sobel_template21 = generate_edge_map(sobel_template2);

    SDoublePlane template_matching2 = cross_correlation(distance_value, sobel_template21);

    vector<DetectedSymbol> notebox2 = templateMatchingThreshold_2(template_matching2, (Type)1);
    allthenoteboxes.insert(allthenoteboxes.end(), notebox2.begin(), notebox2.end());
    write_detection_image("edge2.png", notebox2, input_image);

    SDoublePlane temp3 = templates[2];
    SDoublePlane sobel_hx3 = sobel_gradient_filter(temp3, true);
    SDoublePlane sobel_hy3 = sobel_gradient_filter(temp3, false);
    SDoublePlane sobel_template3 = find_edges(temp3, sobel_hx3, sobel_hy3);

    SDoublePlane sobel_template31 = generate_edge_map(sobel_template3);

    SDoublePlane template_matching3 = cross_correlation(distance_value, sobel_template31);

    vector<DetectedSymbol> notebox3 = templateMatchingThreshold_3(template_matching3, (Type)2);
    allthenoteboxes.insert(allthenoteboxes.end(), notebox3.begin(), notebox3.end());
    write_detection_image("edge3.png", notebox3, input_image);
    write_detection_image("detected5.png", allthenoteboxes, input_image);


	//write_detection_txt("detected_5.txt", allthenoteboxes);
	//write_detection_image("detected_5.png", allthenoteboxes, input_image);

}

std::pair<int, vector<int> > problem6(SDoublePlane &input_image, SDoublePlane &sobel_convolve_seperable_horizontal_output_image, SDoublePlane &sobel_convolve_seperable_vertical_output_image)
{

    //Covert Image to binary Image
	SDoublePlane input_binary_image=convert_image_to_binary(input_image);

	//For debugging purpose only
	SImageIO::write_png_file("binary.png", input_binary_image, input_binary_image, input_binary_image);

	SDoublePlane houghLine(input_binary_image.rows(), input_binary_image.cols());
	SDoublePlane houghStartLine(input_binary_image.rows(), input_binary_image.cols());
	SDoublePlane houghImageOutput(input_binary_image.rows(), input_binary_image.cols());
	SDoublePlane p;
	int diff,valid_lines[100];


	std::pair<int, vector<int> > vals =hough_transform(input_binary_image,sobel_convolve_seperable_horizontal_output_image,sobel_convolve_seperable_vertical_output_image,houghLine);


	return vals;

}

void problem7(const SDoublePlane &input_image, const vector<SDoublePlane> &templates, std::pair<int, vector<int> > lines)
{
	SDoublePlane treble = SImageIO::read_png_file("template_treble.png");

	vector<DetectedSymbol> symbols;
	vector<DetectedSymbol> symbolsAux;
	int threshold = 226;

	for (int i = 0; i < templates.size(); ++i)
	{
		if(i == 0){
			symbolsAux = detectTemplate(input_image, templates.at(i), (Type)(i), threshold);
			int dist = lines.first;
			bool trebleFlag = true;
			for (int j = 0; j < lines.second.size(); ++j)
			{
				int top = lines.second.at(j);
				int bottom = top + 5 * (dist + 1 );


				if(trebleFlag){
					findPitch(symbolsAux, bottom, top, dist, true);
					trebleFlag = false;
				}
				else{

					findPitch(symbolsAux, bottom, top, dist, false);
					trebleFlag = true;
				}

/* this part of code should detect automatically if there is a treble or a bass, but is not working for all the 4 images properly */
				// if(detectBass(input_image, treble, top-2*dist, bottom)){

				// 	findPitch(symbolsAux, bottom, top, dist, true);
				// 	cout << "Treble detected" << endl;
				// }
				// else{

				// 	findPitch(symbolsAux, bottom, top, dist, false);
				// 	cout << "Bass detected" << endl;
				// }

			}
		}
		else{
			symbolsAux = detectTemplate(input_image, templates.at(i), (Type)(i), 240);
		}

		symbols.insert(symbols.end(), symbolsAux.begin(), symbolsAux.end());
	}

	write_detection_txt("detected7.txt", symbols);
	write_detection_image("detected7.png", symbols, input_image);
}





int main(int argc, char *argv[])
{
	if(!(argc == 2))
	{
		cerr << "usage: " << argv[0] << " input_image" << endl;
		return 1;
	}

	string input_filename(argv[1]);

// load images and the template
	SDoublePlane input_image = SImageIO::read_png_file(input_filename.c_str());
	SDoublePlane temp1 = SImageIO::read_png_file("template1.png");
	SDoublePlane temp2 = SImageIO::read_png_file("template2.png");
	SDoublePlane temp3 = SImageIO::read_png_file("template3.png");

// for future use
	vector<SDoublePlane> templates;
	templates.push_back(temp1);
	templates.push_back(temp2);
	templates.push_back(temp3);

/* --------  Problem 2 -------- */
 	// Apply mean Filter NxN
 	cout<<"problem 2"<< endl;
	problem2(input_image);


/* --------  Problem 3 -------- */
 	// Apply gaussian Filter Hx, Hy
	cout<<"problem 3"<< endl;
	problem3(input_image);


/* --------  Problem 4 -------- */
	// Find templates in image
	cout<<"problem 4"<< endl;
	 problem4(input_image, templates, 1);


/* --------  Problem 5 -------- */
    cout<<"problem 5"<< endl;
    problem5(input_image,templates);


/* --------  Problem 6 -------- */
	// apply hough transform
    cout<<"problem 6"<< endl;
	SDoublePlane sobel_convolve_seperable_horizontal_output_image = sobel_hough_gradient_filter(input_image,true);
    SDoublePlane sobel_convolve_seperable_vertical_output_image = sobel_hough_gradient_filter(input_image,false);
    SDoublePlane input_binary_image=convert_image_to_binary(input_image);
    SImageIO::write_png_file("convolve_sobel_vertical_seprable.png", sobel_convolve_seperable_vertical_output_image, sobel_convolve_seperable_vertical_output_image, sobel_convolve_seperable_vertical_output_image);
    std::pair<int, vector<int> > twoVales=problem6(input_binary_image,sobel_convolve_seperable_horizontal_output_image,sobel_convolve_seperable_vertical_output_image);


/* --------  Problem 7 -------- */
	// merge all together
	cout<<"problem 7"<< endl;
	problem7(input_image, templates, twoVales);

}
