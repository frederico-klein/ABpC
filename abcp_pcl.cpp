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

//PointCloud stuff
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>

//debugging stuff
#include <execinfo.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>


using namespace cv;
using namespace std; //idk why i did this if i am just going to write it all the time...

//convert pointcloud from rspoints to pcl_ptr
using pcl_ptr = pcl::PointCloud<pcl::PointXYZ>::Ptr;

bool SAVELATER = true;

std::vector<pcl_ptr> point_vect;
std::vector<Mat> color_d_vect;
std::vector<Mat> color_p_vect;

std::string colorfilename, depthfilename, pointcloudfilename;


static void full_write(int fd, const char *buf, size_t len)
{
        while (len > 0) {
                ssize_t ret = write(fd, buf, len);

                if ((ret == -1) && (errno != EINTR))
                        break;

                buf += (size_t) ret;
                len -= (size_t) ret;
        }
}

void print_backtrace(void)
{
        static const char start[] = "BACKTRACE ------------\n";
        static const char end[] = "----------------------\n";

        void *bt[1024];
        int bt_size;
        char **bt_syms;
        int i;

        bt_size = backtrace(bt, 1024);
        bt_syms = backtrace_symbols(bt, bt_size);
        full_write(STDERR_FILENO, start, strlen(start));
        for (i = 1; i < bt_size; i++) {
                size_t len = strlen(bt_syms[i]);
                full_write(STDERR_FILENO, bt_syms[i], len);
                full_write(STDERR_FILENO, "\n", 1);
        }
        full_write(STDERR_FILENO, end, strlen(end));
    free(bt_syms);
}

pcl_ptr points_to_pcl(const rs2::points& points)
{
    pcl_ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

    auto sp = points.get_profile().as<rs2::video_stream_profile>();
    cloud->width = sp.width();
    cloud->height = sp.height();
    cloud->is_dense = false;
    cloud->points.resize(points.size());
    auto ptr = points.get_vertices();
    for (auto& p : cloud->points)
    {
        p.x = ptr->x;
        p.y = ptr->y;
        p.z = ptr->z;
        ptr++;
    }

    return cloud;
}

// Capture Example demonstrates how to
// capture depth and color video streams and render them to the screen
string askstuff(string, string);
int askstuff(string, int);
double askstuff(string, double);

int main(int argc, char * argv[]) try
{
    //I will acquire same dimensions for depth and colour
    int img_width = 1280;
    int img_height = 720;

    // Create a simple OpenGL window for rendering:
    window app(2*img_width, img_height, "Automatic Brassica-pics Capture ");
    // Declare two textures on the GPU, one for color and one for depth
    texture depth_image, color_image;

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;

    //declaring pointcouldstuff
    rs2::pointcloud pc;
    rs2::points points;

    //configures streams resolution
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, -1, img_width, img_height, rs2_format::RS2_FORMAT_RGB8, 0);
    cfg.enable_stream(RS2_STREAM_DEPTH, -1, img_width, img_height, rs2_format::RS2_FORMAT_ANY, 0);

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    // Start streaming with default recommended configuration
    //pipe.start();
    //starts pipeline with new configuration
    pipe.start(cfg);

    //align?
    rs2::align align_to_depth(RS2_STREAM_DEPTH);
    rs2::align align_to_color(RS2_STREAM_COLOR);

    int i = 0;
    int numofpics = 10;
    double sizeofhead = -5.3;
    std::string pictype;
    pictype = askstuff("Of what are you going to take pictures", "brassica");

    numofpics = askstuff("How many pics do you want",5);

    sizeofhead = askstuff("What's the size of this head",12);

    std::string additional_stuff;
    additional_stuff = askstuff("Anything else to say about the pic?", "");

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
    outfile << "additional_remarks:" + additional_stuff << std::endl;
    outfile.close();

    while(app) // Application still alive?
    {
        rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
        std::cout << "got frames" << std::endl;
        // do i want to align it?
        if (true)
        {
            //either one or the other
            data = align_to_color.process(data);
            //data = align_to_depth.process(data);
        }
        if (false)
        {
            //either one or the other
            //data = align_to_color.process(data);
            data = align_to_depth.process(data);
        }
        std::cout << "got aligned data"<< std::endl;


        rs2::frame depth = data.get_depth_frame(); // Find and colorize the depth data
        // rs2::frame depth = data.get_depth_frame().apply_filter(color_map); // Find and colorize the depth data
        rs2::frame color = data.get_color_frame();            // Find the color data

        std::cout << "initialised frames"<< std::endl;

        // Generate the pointcloud and texture mappings
        points = pc.calculate(depth);
        std::cout << "calculated PointCloud"<< std::endl;

        //convert to pcl
        pcl_ptr pcl_points = points_to_pcl(points);
        std::cout << "converted to pcl"<< std::endl;

        //attempt to colorize it
        rs2::frame depth2 = depth.apply_filter(color_map);

              // Render depth on to the first half of the screen and color on to the second
        depth_image.render(depth2, { 0,               0, app.width() / 2, app.height() });
        color_image.render(color, { app.width() / 2, 0, app.width() / 2, app.height() });

        std::cout << "rendered image"<< std::endl;

        Mat color_p(Size(img_width, img_height), CV_8UC3, (void*)color.get_data(), Mat::AUTO_STEP);
        Mat color_d(Size(img_width, img_height), CV_8UC3, (void*)depth2.get_data(), Mat::AUTO_STEP);

        std::cout << "created opencv matrices"<< std::endl;

        cv::cvtColor(color_p, color_p, CV_BGR2RGB);
        std::cout << "flipped colors"<< std::endl;

        //memory is cheap. time is not
        Mat tempd = color_d.clone();
        Mat tempp = color_p.clone();

        point_vect.push_back(pcl_points);
        color_d_vect.push_back(tempd);
        color_p_vect.push_back(tempp);
        std::cout << "pushed it"<< std::endl;


        try {
            // Define the name of my files to be saved
            //if (i>-11) //first frame always looks ugly, this did not work...
            {
                if(!SAVELATER)
                {
                colorfilename = dirdir + "/pic" + std::to_string(i) + ".png";
                depthfilename = dirdir + "/dep" + std::to_string(i) + ".png";
                pointcloudfilename = dirdir + "/pcl" + std::to_string(i) + ".pcd";

                imwrite(colorfilename, color_p, compression_params);
                imwrite(depthfilename, color_d, compression_params);
                //nofiltering no showing, just save it
                std::cout << "saved pngs"<< std::endl;

                pcl::io::savePCDFile(pointcloudfilename, * pcl_points);
                std::cout << "saved pcd"<< std::endl;
                }

            }
        }
        catch (runtime_error& ex) {
            fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
            return 1;
        }
        i = i + 1;
        if (i > numofpics) break;
    }

    if(SAVELATER)
    {
      //save now!
      for (auto& elem:point_vect)
      {
        auto i = &elem - &point_vect[0];
        colorfilename = dirdir + "/pic" + std::to_string(i) + ".png";
        depthfilename = dirdir + "/dep" + std::to_string(i) + ".png";
        pointcloudfilename = dirdir + "/pcl" + std::to_string(i) + ".pcd";

        imwrite(colorfilename, color_p_vect[i], compression_params);
        imwrite(depthfilename, color_d_vect[i], compression_params);
        //nofiltering no showing, just save it
        std::cout << "saved pngs"<< std::endl;

        pcl::io::savePCDFile(pointcloudfilename, * point_vect[i]);
        std::cout << "saved pcd"<< std::endl;
      }
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
    print_backtrace();
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
