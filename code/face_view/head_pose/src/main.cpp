//============================================================================
// Name        : main.cpp
// Author      : Varsha
// Version     : v1
// Copyright   : 
// Description : Head pose output for an input of single frames
//============================================================================


#include <iostream>
#include <sstream>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cvaux.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "head_pose.hpp"

using namespace cv;
using namespace std;

#define DEPTH_XRES 640
#define DEPTH_YRES 480

void get_head_pose_estimate(Mat &depth_mat) {
    int valid_pixels = 0;

    //    generate 3D image for head pose
    for (int y = 0; y < g_im3D.rows; y++) {
        Vec3f* Mi = g_im3D.ptr<Vec3f > (y);
        for (int x = 0; x < g_im3D.cols; x++) {
            float d = (ushort) depth_mat.at<ushort > (y, x);
            if (d < g_max_z && d > 0) {
                valid_pixels++;
                Mi[x][0] = (float(d * (x - 320)) / f);
                Mi[x][1] = (float(d * (y - 240)) / f);
                Mi[x][2] = d;
            } else
                Mi[x] = 0;
        }
    }
    
    g_means.clear();
    g_votes.clear();
    g_clusters.clear();

    //run estimation
    g_Estimate->estimate(g_im3D, g_means, g_clusters, g_votes, g_stride, g_maxv, g_prob_th, g_larger_radius_ratio, g_smaller_radius_ratio, false, g_th);
}

void init_headpose() {
    // load head pose config file and trees
    loadConfig("../config.txt");
    g_Estimate = new CRForestEstimator();
    if (!g_Estimate->loadForest(g_treepath.c_str(), g_ntrees)) {

        cerr << "could not read forest!" << endl;
        exit(-1);
    }

    // ZPS = zero plane distance = g_focal_length in mm - predefined
    // ZPPS = pixel size at zero plane = g_pixel_size in mm - predefined
    g_pixel_size *= 2.f;
    f = g_focal_length / g_pixel_size;

    g_im3D.create(DEPTH_YRES, DEPTH_XRES, CV_32FC3);
}

int main(int argc, const char * argv[]) {

    string output_file_path = "/fiddlestix/Users/varsha/Documents/ResearchEyetrackCode/eyetrack/code/face_view/head_pose/head_pose_output.txt";
    string input_file_path = "/fiddlestix/Users/varsha/Documents/ResearchEyetrackCode/eyetrack/all_images/test/depth_file_list.txt";

    
    
    // initialise head pose setup
    init_headpose();

    ifstream fin(input_file_path.c_str());
    ofstream fout(output_file_path.c_str());
    // check for bogus paths
    if (!fin.is_open()) {
        cout << "Could not open input file : " << input_file_path << endl;
        return 1;
    }
    if (!fout.is_open()) {
        cout << "Could not open output file : " << output_file_path << endl;
        return 1;
    }

    string depth_file_name;
    while (fin.good()) {
        getline(fin, depth_file_name);
        fout << depth_file_name << ",";
        ifstream fin(depth_file_name.c_str(), ios::binary | ios::ate);
        if (fin.is_open()) {
            // create a depth cv::mat
            ifstream::pos_type size = fin.tellg();
            fin.seekg(0, ios::beg);
            char *data = new char[size];
            fin.read(data, size);
            fin.close();
            Mat depth_mat(DEPTH_YRES, DEPTH_XRES, CV_16UC1, (void*) data);
            // run head pose estimation
            get_head_pose_estimate(depth_mat);
            // write to file if valid pose is returned
            if (g_means.size() > 0) {
                fout << g_means[0][3] << "," << g_means[0][4] << "," << g_means[0][5] << "," << g_means[0][0] << "," << g_means[0][1] << "," << g_means[0][2] << endl;
                cout << "SUCCESS Processed : " << depth_file_name << endl;
            } else {
                fout << endl;
                cout << "FAILED Processed : " << depth_file_name << endl;
            }
            delete[] data;
        } else
            cout << "Failed to open :" << depth_file_name << endl;
    }
    return 0;
}
