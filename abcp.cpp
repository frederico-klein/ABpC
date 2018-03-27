// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <ctime>
// to create the metadata file
#include <iostream>
#include <fstream>
// to manipulate all the strings...
#include <sstream>
#include <boost/lexical_cast.hpp>

using namespace cv;
using namespace std; //idk why i did this if i am just going to write it all the time...

// Capture Example demonstrates how to
// capture depth and color video streams and render them to the screen
string askstuff(string, string);
int askstuff(string, int);
double askstuff(string, double);

int main(int argc, char * argv[]) try
{

    // Create a simple OpenGL window for rendering:
    window app(1280, 720, "Automatic Brassica-pics Capture ");
    // Declare two textures on the GPU, one for color and one for depth
    texture depth_image, color_image;

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    // Start streaming with default recommended configuration
    pipe.start();

    int i = 0;
    int numofpics = 10;
    double sizeofhead = -5.3;
    std::string pictype;
    pictype = askstuff("Of what are you going to take pictures", "brassica");

    numofpics = askstuff("How many pics do you want",10);

    sizeofhead = askstuff("What's the size of this head",2.7);

    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

    //creates a directory with the current timestamp. maybe hard to follow, but i don't want to create a function to look at all directories and create the next one
    std::time_t result = std::time(nullptr);
    std::string dirdir = std::asctime(std::localtime(&result));
    dirdir.pop_back();
    const int dir_err = mkdir(dirdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (-1 == dir_err)
    {
        printf("Error creating directory!n");
        exit(1);
    }

    // my directory exists, I can create a metadata file now
    std::ofstream outfile (dirdir + "/metadata.txt");
    outfile << "picsof:" + pictype << std::endl;
    outfile << "numpics:" + std::to_string(numofpics) << std::endl;
    outfile << "sizeofhead:" + std::to_string(sizeofhead) << std::endl;
    outfile.close();

    while(app) // Application still alive?
    {
        rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera

        rs2::frame depth = color_map(data.get_depth_frame()); // Find and colorize the depth data
        rs2::frame color = data.get_color_frame();            // Find the color data

              // Render depth on to the first half of the screen and color on to the second
        depth_image.render(depth, { 0,               0, app.width() / 2, app.height() });
        color_image.render(color, { app.width() / 2, 0, app.width() / 2, app.height() });


        Mat color_p(Size(640, 480), CV_8UC3, (void*)color.get_data(), Mat::AUTO_STEP);
        Mat color_d(Size(1280, 720), CV_8UC3, (void*)depth.get_data(), Mat::AUTO_STEP);

        cv::cvtColor(color_p, color_p, CV_BGR2RGB);

        try {
            // Define the name of my files to be saved
            //if (i>-11) //first frame always looks ugly, this did not work...
            {
                std::string colorfilename = dirdir + "/pic" + std::to_string(i) + ".png";
                std::string depthfilename = dirdir + "/dep" + std::to_string(i) + ".png";
                imwrite(colorfilename, color_p, compression_params);
                imwrite(depthfilename, color_d, compression_params);
            }
        }
        catch (runtime_error& ex) {
            fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
            return 1;
        }
        i = i + 1;
        if (i > numofpics) break;
    }

//    rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
//    rs2::frame color = data.get_color_frame();            // Find the color data

    // Creating OpenCV Matrix from a color image
       // Mat color_p(Size(640, 480), CV_8UC3, (void*)color.get_data(), Mat::AUTO_STEP);

        // Display in a GUI
        //namedWindow("Display Image", WINDOW_AUTOSIZE );
        //imshow("Display Image", color_p);

          //  waitKey(0);

    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

string askstuff(string question, string defaultvalue)
{
    std::string myanswer;
    std::string fullquestion = question + "? (Default:"+ defaultvalue +")\nHit <ENTER> to terminate label\n";
    printf("%s",fullquestion.c_str());
    std::getline(cin, myanswer);
    if (myanswer.empty())
    {
    myanswer = defaultvalue;
    }
    return myanswer;
}

int askstuff(string question, int defaultvalue) //Im pretty sure im overcomplicating this
{
    std::string myanswer;
    int mynumanswer;
    std::string fullquestion = question + "? (Default:"+ std::to_string(defaultvalue) +")\nHit <ENTER> to terminate label\n";
    printf("%s",fullquestion.c_str());
    std::getline(cin, myanswer);
    stringstream geek(myanswer);
    if (myanswer.empty())
    {
    mynumanswer = defaultvalue;
    }
    else
    {
    geek >> mynumanswer;
    }
    return mynumanswer;
}

double askstuff(string question, double defaultvalue) //Im pretty sure im overcomplicating this
{
    std::string myanswer;
    float mynumanswer;
    std::string fullquestion = question + "? (Default:"+ std::to_string(defaultvalue) +")\nHit <ENTER> to terminate label\n";
    printf("%s",fullquestion.c_str());
    std::getline(cin, myanswer);
    if (myanswer.empty())
    {
    mynumanswer = defaultvalue;
    }
    else
    {
    mynumanswer = boost::lexical_cast<float>(myanswer);
    }
    return mynumanswer;
}
