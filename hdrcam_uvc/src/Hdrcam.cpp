/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Hdrcam.cpp
 * Author: esailly
 *
 * Created on 26 avril 2017, 10:28
 */
#include <opencv2/opencv.hpp>
#include <libuvc/libuvc.h>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include "Variables.hpp"

// uvc data (webcam)
uvc_context_t * gCxt;
uvc_device_handle_t * gDevh;
uvc_device_t * gDev;
uvc_stream_ctrl_t gCtrl;
uvc_frame_t * gFrame;

// image parameters
int gWidth = 640;
int gHeight = 480;
int imgIndex = 0;
Data * gData; 

std::mutex gMutex;

/** @brief call uvc function and check return code
 * @param msg pointeur on error message
 * @param res ??
 */ 
void tryUvc(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        exit(-1);
    }
}

/** @brief Callback function for ExpMin tarckBar events.
values must be between 30 and 1000
@param _min is the value of trackBar Min.
@param d is a pointer of data structure se Data
 */
void handleTrackbarMin(int _min, void * d) {
    Data * _data = (Data *) d;
    
    _data->setExpMin(_min);
    cv::setTrackbarPos("Exp_min_(ms)", "MonHDR",_data->expMin);
    cv::setTrackbarPos("Exp_max_(ms)", "MonHDR",_data->expMax);    
    cv::setTrackbarPos("Images", "MonHDR", _data->nbImg);
    
    cv::imshow("MonHDR", _data->img);
}

/** @brief Callback function for ExpMax tarckBar events.
 values must be between 60 and 1000
 @param _max is the value of trackBar Min.
 @param d is a pointer of data structure se Data
 */
void handleTrackbarMax(int _max, void * d) {
    Data * _data = (Data *) d;
    
     _data->setExpMax(_max);
    cv::setTrackbarPos("Exp_max_(ms)", "MonHDR",_data->expMax);
    cv::setTrackbarPos("Exp_min_(ms)", "MonHDR",_data->expMin);
    cv::setTrackbarPos("Images", "MonHDR", _data->nbImg);
    
    cv::imshow("MonHDR", _data->img);
}

/** @brief Callback function for Images tarckBar events.
  values must be between 2 and 5 
 @param _nb is the value of trackBar Images.
 @param d is a pointer of data structure se Data
 */
void handleTrackbarNb(int _nb, void * d) {
    Data * _data = (Data *) d;
    _data->setNbImg(_nb);
    cv::setTrackbarPos("Images", "MonHDR", _data->nbImg);
    cv::setTrackbarPos("Exp_max_(ms)", "MonHDR",_data->expMax);
    cv::setTrackbarPos("Exp_min_(ms)", "MonHDR",_data->expMin);
    
    cv::imshow("MonHDR", _data->img);
}

// prevent errors in console output
int handleError(int, const char *, const char *, const char *, int, void *) {
    return 0;
}

// capture callback function
void captureFun(uvc_frame_t * frame, void *) {
    // prevent concurrency (image processing)
    std::lock_guard<std::mutex> lock(gMutex);
    // get image from webcam
    tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));
    // change exposure time for next capture
    
    //tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gData->expMin*10));
    
    imgIndex = (imgIndex < gData->nbImg-1) ? imgIndex + 1 : 0 ;
    
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gData->exp[imgIndex]*10));
    
   
    
//    if (gIsDark) { // Dark to Bright
//        gIsDark   = false;
//        gIsBright = true;
//        tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gBrightExp));
//    } else if (gIsBright) { // Bright to Medium
//                gIsBright = false;
//                gIsMedium = true;
//                tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gMediumExp)); 
//            } else { // Medium to Dark
//                gIsMedium = false;
//                gIsDark   = true;
//                tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gDarkExp));
//            }
}


/*
 * 
 */
int main(int argc, char** argv) {
    
    
    
    cv::namedWindow("MonHDR");
    
    cv::namedWindow("image0");
    cv::namedWindow("image1");
    cv::namedWindow("image2");
    
    cv::moveWindow("MonHDR", 0, 0);
    
    cv::redirectError(handleError);
    Data data; // data for trackers
    data.init();
   
    
    gData = &data;
    
    std::cout << "temps d'expo " <<  gData->exp[imgIndex] << std::endl;
    //PictureHdr images(data.expMin);
    
    
    
    
    cv::Mat image(gHeight,gWidth,CV_8UC3,{255,255,255});

    
    data.img = image;
    cv::createTrackbar("Exp_min_(ms)", "MonHDR", &data.expMin , 1000, handleTrackbarMin, &data);
    cv::createTrackbar("Exp_max_(ms)", "MonHDR", &data.expMax, 1000, handleTrackbarMax, &data);
    cv::createTrackbar("Images", "MonHDR", &data.nbImg, 5, handleTrackbarNb, &data);
    handleTrackbarMin(30, &data);

    cv::imshow("MonHDR", image);
     
    
    // init uvc data (webcam)
    tryUvc("init", uvc_init(&(gCxt), nullptr));
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 0, nullptr));
    tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 2057, nullptr)); //logitec
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 12314, nullptr)); //hercules
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 19911, nullptr));
    
    tryUvc("open", uvc_open(gDev, &(gDevh)));
    tryUvc("get_ctrl", uvc_get_stream_ctrl_format_size(gDevh, 
                &(gCtrl), UVC_FRAME_FORMAT_ANY, gWidth, gHeight, 30));
    tryUvc("set_ae_mode", uvc_set_ae_mode(gDevh, 1));
   
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, 300));
    gFrame = uvc_allocate_frame(gWidth * gHeight * 3);
    if (not gFrame) {
        std::cerr << "unable to allocate frame!\n";
        exit(-1);
    }
    
    
    //Start visualisation
    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
                &(gCtrl), captureFun, nullptr, 0));
    
    try {
        // terminate when window is closed
        int cpt = 0;
       // int grayAvg = 0;
        while (cv::getWindowProperty("MonHDR", 0) != -1) {
            // terminate when <esc> is hit
            if (cv::waitKey(30) % 0x100 == 27) {
                cv::destroyAllWindows();
            } else {
                //Windows view streaming
                // prevent concurrency (webcam capture)
                std::lock_guard<std::mutex> lock(gMutex); 
                
                // process new frame
                cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
                
                data.img = mat;        
                
                if (imgIndex<data.nbImg) {
                    data.pictures[imgIndex] = mat.clone();

                    //std::cout <<  " image"+ std::to_string(imgIndex);             
                    cv::imshow("image"+std::to_string(imgIndex), data.pictures[imgIndex]);

                    if (cpt > 25) {
//                        grayAvg = data.getImageAvg(imgIndex);
//
//                        if (imgIndex == 0) { //first
//                            if (grayAvg < data.mAvg[imgIndex] ) {
//                                cv::setTrackbarPos("Exp_min_(ms)", "MonHDR",data.expMin * 0.9);
//                            } else {
//                                cv::setTrackbarPos("Exp_min_(ms)", "MonHDR",data.expMin * 1.1);
//                            }                        
//                        } else {
//                            if (imgIndex == data.nbImg -1) { //last
//                               if (grayAvg < data.mAvg[imgIndex] ) {
//                                    cv::setTrackbarPos("Exp_max_(ms)", "MonHDR",data.expMax * 0.9);
//                                } else {
//                                    cv::setTrackbarPos("Exp_max_(ms)", "MonHDR",data.expMax * 1.1);
//                                }                             
//                            } else { //middle
//                               //nothing to do 
//                            }
//                        } 
                        
                        cv::Ptr<cv::CalibrateDebevec> calibrateDebevec = cv::createCalibrateDebevec();
                        calibrateDebevec->process(data.pictures, data.reponse, data.times);
                        cpt =  0;
                    }

                }
                
                if (!data.reponse.empty()) {
                    cv::Mat hdr;
                    cv::Mat exr;
                    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
                    merge_debevec->process(data.pictures, hdr, data.times, data.reponse);
                    cv::normalize(hdr, exr, 0, 1, cv::NORM_MINMAX);
                    cv::pow(exr, 0.2, exr); //data->gamma/100.0
                    exr *= 255.0;// *.5;//* data->coef/100.0;
                    cv::imshow("MonHDR", exr); 
                }
                //cv::imshow("MonHDR", data.img);
                
                
                //std::cout << " IMG" << gData->exp[imgIndex]*10;
                cpt++;
            }
        }
    }
    catch (cv::Exception&) {
    }
    
    std::cout << "Je suis la ! " << std::endl;
    return 0;
}

