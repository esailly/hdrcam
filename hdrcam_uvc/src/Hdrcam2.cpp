/* 
 * File:   Hdrcam2.cpp
 * Author: esailly
 * Fork Code de base J Dehos
 * Created on 3 mai 2017, 13:48
 */


#include <opencv2/opencv.hpp>
#include <libuvc/libuvc.h>
#include <mutex>
#include <cstdlib>
#include <iostream>
#include "Variables.hpp"
#include "Chrono.hpp"
#include <string>

// uvc data (webcam)
uvc_context_t * gCxt;
uvc_device_handle_t * gDevh;
uvc_device_t * gDev;
uvc_stream_ctrl_t gCtrl;
uvc_stream_handle_t *gStrmh;
uvc_frame_t * gFrame;
uint32_t aTime = 0;
Chrono gChrono;

// image parameters
int gWidth = 640;
int gHeight = 480;
int imgIndex = 0;
int imgIndex2 = 0;

int cptInt = 0;
int cptFun = 0;

Data gData; 

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


// capture callback function
void captureFun(uvc_frame_t * frame, void *) {
    // prevent concurrency (image processing)
    std::lock_guard<std::mutex> lock(gMutex);    

    if (gData.nbframe > 0 ) {  
        // get image from webcam
        tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));
               
        //copy frame to Vector Pictures
        cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
        mat.copyTo(gData.pictures[imgIndex]);

        // change exposure time for next capture    
        imgIndex = (imgIndex + 1) % gData.nbImg;
        tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gData.exp[imgIndex]));

        gChrono.restart();
        gData.nbframe =0;
        
    } else gData.nbframe++ ;
    
}

void printImageStats(const cv::Mat & img) {
    std::cout << "cols: " << img.cols << std::endl;
    std::cout << "rows: " << img.rows << std::endl;
    std::cout << "channels: " << img.channels() << std::endl;

    std::cout << "depth: ";
    switch (img.depth()) {
        case CV_8U: std::cout << "CV_8U" << std::endl; break;
        case CV_8S: std::cout << "CV_8S" << std::endl; break;
        case CV_16U: std::cout << "CV_16U" << std::endl; break;
        case CV_16S: std::cout << "CV_16S" << std::endl; break;
        case CV_32S: std::cout << "CV_32S" << std::endl; break;
        case CV_32F: std::cout << "CV_32F" << std::endl; break;
        case CV_64F: std::cout << "CV_64F" << std::endl; break;
        default: std::cout << "unknown format" << std::endl;
    }

    double minVal, maxVal; 
    cv::minMaxIdx(img, &minVal, &maxVal);
    std::cout << "min: " << minVal << std::endl;
    std::cout << "max: " << maxVal << std::endl;

    cv::Scalar meanScalar = cv::mean(img);
    std::cout << "mean: " << meanScalar << std::endl;
}


cv::Mat calculImageHdr(std::vector<cv::Mat> _imgages, std::vector<float> _times);

/*
 * 
 */
int main(int argc, char** argv) {
    int pexpmin = 0;
    int pexpmax = 0;
    int pnb = 0;
    
    if (argc > 1) {
        int cpt = 1;
        while (cpt < argc) {
           std::string s =  argv[cpt];
           if(s.find("-min=") != std::string::npos){
             pexpmin =stoi(s.substr(5));  
           } 
           
           if(s.find("-max=") != std::string::npos){
             pexpmax =stoi(s.substr(5));  
           }
           
           if(s.find("-nb=") != std::string::npos){
             pnb =stoi(s.substr(4));  
           }
           cpt++;
        }        
    } 
    
    if (pexpmin == 0 || pexpmax == 0 || pnb == 0 || pnb > 5 ){
        std::cerr << "usage: " << argv[0] << " -min=xx -max=xx -nb=2..5 \n"
               << "min exposition time in ms \n max exposition time in ms \n"
               << "nb number of pictures for HDR \n"
               << "execute with param by default -min=30 -max=60 -nb=2 \n";
        pnb=2;
        pexpmin=30;
        pexpmax=60;
    } 
    
    std::cout << "Demarrage de l'application avec les params nb:" << pnb 
              << " min:" << pexpmin << " max:" << pexpmax <<std::endl;

    cv::namedWindow("MonHDR");
    
    //Data data; 
    gData.init(pexpmin,pexpmax,pnb);
    //gData = &data;
    //imgIndex = data.nbImg-1;
    
    cv::Mat image(gHeight,gWidth,CV_8UC3,{255,255,255});
    gData.img = image;
    cv::imshow("MonHDR", image);
    cv::moveWindow("MonHDR", 0, 0);
    
    // init uvc data (webcam)
    tryUvc("init", uvc_init(&(gCxt), nullptr));
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 0, nullptr));
    
    tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 2057, nullptr)); //logitec
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 12314, nullptr)); //hercules
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 19911, nullptr));
    
    tryUvc("open", uvc_open(gDev, &(gDevh)));
    
    tryUvc("get_ctrl", uvc_get_stream_ctrl_format_size(gDevh, 
                &(gCtrl), UVC_FRAME_FORMAT_ANY, gWidth, gHeight, 30));
    
    tryUvc("set_ae_mode", uvc_set_ae_mode(gDevh, 1)); // origin  1
             
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gData.expMin));
             
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //tryUvc("stream_open_ctrl", uvc_stream_open_ctrl(gDevh, &gStrmh, &gCtrl));
    
        gFrame = uvc_allocate_frame(gWidth * gHeight * 3);
        //aFrame = uvc_allocate_frame(gWidth * gHeight * 3);
    if (not gFrame) {
        std::cerr << "unable to allocate frame!\n";
        exit(-1);
    }
    gChrono.restart();    
    //Start visualisation
    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
                &(gCtrl), captureFun, nullptr , 0)); //
    
        
   
   
    try {
        
        int idx;

        // terminate when window is closed
        while (cv::getWindowProperty("MonHDR", 0) != -1) {
            //Windows view streaming
            
            // prevent concurrency (webcam capture)
            std::lock_guard<std::mutex> lock(gMutex);             
            
            switch (cv::waitKey(30) % 0x100) {
                case 27: // terminate when <esc> is hit
                    cv::destroyAllWindows();
                    std::cout << "fermeture" << std::endl;
                    break;
                case 32: // terminate when <Space> is hit "space bar"
                    cv::imshow("MonHDR", calculImageHdr(gData.pictures,gData.times)); 
                    break;
                case 48: // terminate when <0> 
                    cv::imshow("MonHDR", gData.pictures[0]); 
                    break;    
                case 49: // terminate when <0> 
                    cv::imshow("MonHDR", gData.pictures[1]); 
                    break;
                case 50: // terminate when <0> 
                    if (gData.nbImg > 2 ) {
                    cv::imshow("MonHDR", gData.pictures[2]); 
                    } 
                    break;
                case 51: // terminate when <0> 
                    if (gData.nbImg > 3 ) {
                    cv::imshow("MonHDR", gData.pictures[3]); 
                    }
                    break;
                case 52: // terminate when <0> 
                    if (gData.nbImg > 4 ) {
                    cv::imshow("MonHDR", gData.pictures[4]); 
                    } 
                    break;
                default:
                    
                     if (gChrono.elapsedRunning()*10 > gData.exp[imgIndex] )  {
                        cv::imshow("img"+std::to_string(imgIndex)+"_"+std::to_string(gData.exp[imgIndex]),gData.pictures[imgIndex]); 
                     }
                     //  std::cout << imgIndex << " "  << " elapsed " << gChrono.elapsedRunning() << std::endl;
            }
        }
    }
    catch (cv::Exception&) {
    }
       
    return 0;
}

cv::Mat calculImageHdr(std::vector<cv::Mat> _images, 
                       std::vector<float> _times) {
    
    cv::Mat image = _images[0].clone();
    std::vector<cv::Mat> images; 
    std::vector<float> times;
    for (auto img : _images) {
        images.push_back(img.clone());
    }
    for (auto t : _times) {
        times.push_back(t);
    }
    
    
    gData.printTimes();
    
    
    cv::Mat reponse;    
    cv::Ptr<cv::CalibrateDebevec> calibrateDebevec = cv::createCalibrateDebevec();
    calibrateDebevec->process(images, reponse, times);
    
    
    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
    merge_debevec->process(images, image, times, reponse);
    
    cv::Mat imageldr;
    cv::Ptr<cv::Tonemap> tonemap1 = cv::createTonemap(2.2f);
    tonemap1->process(image, imageldr);
        
    
//    cv::normalize(image, imageldr, 0, 1, cv::NORM_MINMAX);
//    cv::pow(imageldr, 2.2f, imageldr);
//    imageldr *= 255.0 * 1.0f;
    
    return imageldr;
    
}




