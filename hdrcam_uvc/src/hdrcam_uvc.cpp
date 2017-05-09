#include <opencv2/opencv.hpp>
#include <libuvc/libuvc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <thread>

// uvc data (webcam)
uvc_context_t * gCxt;
uvc_device_handle_t * gDevh;
uvc_device_t * gDev;
uvc_stream_ctrl_t gCtrl;
uvc_frame_t * gFrame;

// image parameters
int gWidth = 640;
int gHeight = 480;

int gDarkExp = 100;
int gBrightExp = gDarkExp*8;
int gMediumExp = gDarkExp*4;



std::vector<cv::Mat> images(3);
std::vector<float> times(3);

// capture/processing synchronization
bool gIsDark   = false;
bool gIsBright = true;
bool gIsMedium = false;
std::mutex gMutex;

// call uvc function and check return code
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
    // get image from webcam
    tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));
    // change exposure time for next capture
    if (gIsDark) { // Dark to Bright
        gIsDark   = false;
        gIsBright = true;
        tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gBrightExp));
    } else if (gIsBright) { // Bright to Medium
                gIsBright = false;
                gIsMedium = true;
                tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gMediumExp)); 
            } else { // Medium to Dark
                gIsMedium = false;
                gIsDark   = true;
                tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gDarkExp));
            }
}

// capture callback function
void captureCalibrage(uvc_frame_t * frame, void *) {
    // prevent concurrency (image processing)
    std::lock_guard<std::mutex> lock(gMutex);
    // get image from webcam
    tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));
    // change exposure time for next capture
    if (gIsDark) gIsDark = false;
    if (gIsBright) gIsBright = false; 
    if (gIsMedium) gIsMedium = false;        
}

void loadExposureSeq(std::string, std::vector<cv::Mat>&, std::vector<float>&);
void saveResponse(std::string, cv::Mat &);

int main() {

    // init opencv data (image processing and UI)
    cv::namedWindow("dark");
    cv::moveWindow("dark", 0, 550);
    cv::namedWindow("bright");
    cv::moveWindow("bright", 1280, 550);
    cv::namedWindow("medium");
    cv::moveWindow("medium", 640, 550);
   // cv::namedWindow("HDR");
   // cv::namedWindow("HDR1");
    cv::namedWindow("HDRDebevec");
    cv::moveWindow("HDRDebevec", 0, 0);
    cv::namedWindow("HDRRobertson");
    cv::moveWindow("HDRRobertson", 640, 0);
    cv::namedWindow("HDR_fusion");
    cv::moveWindow("HDR_fusion", 1280, 0);
    
    
    // init uvc data (webcam)
    tryUvc("init", uvc_init(&(gCxt), nullptr));
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 0, nullptr));
    tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 2057, nullptr)); //logitec
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 12314, nullptr));
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 19911, nullptr));
    tryUvc("open", uvc_open(gDev, &(gDevh)));
    tryUvc("get_ctrl", uvc_get_stream_ctrl_format_size(gDevh, 
                &(gCtrl), UVC_FRAME_FORMAT_ANY, gWidth, gHeight, 30));
    tryUvc("set_ae_mode", uvc_set_ae_mode(gDevh, 1));
   
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, 100));
    gFrame = uvc_allocate_frame(gWidth * gHeight * 3);
    if (not gFrame) {
        std::cerr << "unable to allocate frame!\n";
        exit(-1);
    }
    
//    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
//                &(gCtrl), captureFun, nullptr, 0));
    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
                &(gCtrl), captureCalibrage, nullptr, 0));
    
    
    times[0] =  (float)gBrightExp / (float) 10000; // 800/10000 = 0.08 
    times[1] =  (float)gMediumExp / (float) 10000; // 400/10000 = 0.04
    times[2] =  (float)gDarkExp / (float) 10000;   // 100/10000 = 0.01
        
    //calibrage webcam
    cv::Mat reponseDebevec;
    cv::Mat reponseRobertson;
    
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gDarkExp));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    gIsDark = true;
    while (gIsDark) {
       std::lock_guard<std::mutex> lock(gMutex);
       cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
       images[2] = mat.clone();
       //std::cout << "dark ";
    }
    
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gMediumExp));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    gIsMedium = true;
    while (gIsMedium) {
       std::lock_guard<std::mutex> lock(gMutex);
       cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
       images[1] = mat.clone();
      // std::cout << "medium ";
    }
    
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gBrightExp));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    gIsBright = true;
    while (gIsBright) {
       std::lock_guard<std::mutex> lock(gMutex);
       cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
       images[0] = mat.clone();
      // std::cout << "bright ";
    }
    uvc_stop_streaming(gDevh);
    
//
//    cal_debvec = cv2.createCalibrateDebevec()
//    crf_debvec = cal_debvec.process(img_list, times=exposure_times)
//    hdr_debvec = merge_debvec.process(img_list, times=exposure_times.copy(), response=crf_debvec.copy())
//    cal_robertson = cv2.createCalibrateRobertson()
//    crf_robertson = cal_robertson.process(img_list, times=exposure_times)
//    hdr_robertson = merge_robertson.process(img_list, times=exposure_times.copy(), response=crf_robertson.copy())
    
    cv::Ptr<cv::CalibrateDebevec> calibrateDebevec = cv::createCalibrateDebevec();
    calibrateDebevec->process(images, reponseDebevec, times);
    //std::cout << reponse;  
    saveResponse("/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/CalDebevec.csv",reponseDebevec);
    
    cv::Mat hdr;
    
    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
    merge_debevec->process(images, hdr, times, reponseDebevec);
    
    cv::Mat ldr;
    cv::Ptr<cv::TonemapDurand> tonemap1 = cv::createTonemapDurand(2.2f);
    tonemap1->process(hdr, ldr);
    
    cv::imshow("HDRDebevec", ldr);
    
    
    cv::Ptr<cv::CalibrateRobertson> calibrateRobertson = cv::createCalibrateRobertson();
    calibrateRobertson->process(images, reponseRobertson, times);
    saveResponse("/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/CalRobertson.csv",reponseRobertson);
    
    cv::Mat hdr2;    
    cv::Ptr<cv::MergeRobertson> merge_Robertson = cv::createMergeRobertson();
    merge_Robertson->process(images, hdr2, times, reponseRobertson);
    
    cv::Mat ldr2;
    cv::Ptr<cv::TonemapDurand> tonemap2 = cv::createTonemapDurand(2.2f);
    tonemap2->process(hdr2, ldr2);
    cv::imshow("HDRRobertson", ldr2);
    
    
//    cv::Mat ldr;
//    cv::Ptr<cv::TonemapDurand> tonemap = cv::createTonemapDurand(2.2f);
//    tonemap->process(hdr, ldr2);  
//    ldr2 = ldr2*255;
//    cv::imshow("HDR1", ldr2);
    
    cv::Mat fusion;
    cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
    merge_mertens->process(images, fusion);
    
    cv::imshow("HDR_fusion", fusion);
    
    cv::imshow("dark", images[2]);
    
    cv::imshow("medium", images[1]);
    cv::imshow("bright", images[0]);
    
    
    
    //tart visualisation
//    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
//                &(gCtrl), captureFun, nullptr, 0));
   
    // main loop
    while (true) {

        // handle user events
        int key = cv::waitKey(30);
        if (key != 255) std::cout << key << " " ;
        if (key % 0x100 == 27) 
            break;
        
        /*       
        // get and display webcam frame
        // prevent concurrency (webcam capture)
        std::lock_guard<std::mutex> lock(gMutex);  
        // process new frame
        cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
        
               
                
        if (gIsDark) {
            images[2] = mat.clone();
            cv::imshow("dark", mat);
         } else if (gIsMedium) { 
                 images[1] = mat.clone();
                 cv::imshow("medium", mat);
                } else {
                       images[0] = mat.clone();
                       cv::imshow("bright", mat);
                }
     */ 
    }
   

    /*
    // free uvc data -> BUG ???
    uvc_free_frame(gFrame);
    uvc_stop_streaming(gDevh);
    uvc_close(gDevh);
    uvc_unref_device(gDev);
    uvc_exit(gCxt);
    */

    return 0;
}

void loadExposureSeq(std::string path, std::vector<cv::Mat>& images, std::vector<float>& times)
{
    path = path + std::string("/");
    std::ifstream list_file((path + "list.txt").c_str());
    std::string name;
    float val;
    while(list_file >> name >> val) {
        cv::Mat img = cv::imread(path + name);
        images.push_back(img);
        times.push_back(1 / val);
    }
    list_file.close();
}

void saveResponse(std::string filename, cv::Mat & response) {
   std::ofstream myfile (filename, std::ios::out);

   myfile << "\"b\" \"g\" \"r\"" << std::endl;
   for (int i = 0; i < response.rows; i++) {
        myfile << response.at<cv::Vec3f>(i,0)[0] << " " 
           << response.at<cv::Vec3f>(i,0)[1] << " "
           << response.at<cv::Vec3f>(i,0)[2] << std::endl;
 //        std::cout << response.at<cv::Vec3f>(i,0)[0] << " " 
//                 << response.at<cv::Vec3f>(i,0)[1] << " "
//                 << response.at<cv::Vec3f>(i,0)[2] << std::endl;
    }   
   myfile.close();
}

//cv::Mat ImageHdrDebevec() {
//    
//} 