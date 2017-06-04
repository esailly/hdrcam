/*
 * Copyright 2017 esailly.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   HdrCamVideo.cpp
 * Author: esailly
 *
 * Created on 18 mai 2017, 15:21
 */

#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <libuvc/libuvc.h>
#include <mutex>
#include <vector>
#include <iostream>
#include "Chrono.hpp"
#include "Variables.hpp"

// uvc data (webcam)
uvc_context_t * gCxt;
uvc_device_handle_t * gDevh;
uvc_device_t * gDev;
uvc_stream_ctrl_t gCtrl;
uvc_stream_handle_t *gStrmh;
uvc_frame_t * gFrame;



Chrono ChronoExp;
Chrono tempsTestChrono;
// image parameters
int gWidth = 640;
int gHeight = 480;
int imgIndex = 0;

Data gData; 
Data dataCal; 

bool capture = false;
std::mutex gMutex;


// predeclaration des fonctions
void tryUvc(const char *, uvc_error_t);
uvc_error_t tryUvcRet(const char *, uvc_error_t);

//predeclarations  
void captureStd(uvc_frame_t * frame, void *); // pour streaming
void captureCal(uvc_frame_t *, void *); // pour calibrage 
cv::Mat calibrageCam(int);
void col2Gray(const cv::Mat & imgCo, cv::Mat &imgGray );


void afficheArg() {
    std::cout << "Usage: " << "HdrCam_Video.out" << " <Methode> [Filename] [path] " << std::endl;
    std::cout << "L'argument Methode est un entier avec les valeurs suivantes : \n1 : Debevec\n2 : Robertson \n0 : Mertens " << std::endl;
    std::cout << "si vous renseignez un nom de fichier il sauvegardera les images." << std::endl; 
    std::cout << "L'argument Filename est le nom que vous voulez donner au fichier." << std::endl;
    std::cout << "Exemple Window créera Calibrage_window_Deb(0).png " << std::endl;
    std::cout << "L'argument path est la localisation des fichiers de sauvegarde" << std::endl;
    std::cout << "\nle programme démarre en mode Bright la touche l switch Bright/Dark" << std::endl;
    std::cout << "la touche s sauvegarde les images visionées." << std::endl;
    std::cout << "la touche . ferme les fenetres sauf la principale." << std::endl;
    std::cout << "la touche 0 affiche les images de base pour le calcul." << std::endl;
    std::cout << "la touche 5 passe en Gris ou revient en Couleur." << std::endl; 
}


/*
 * 
 */
int main(int argc, char** argv) {
    
    std::string filename = "test";
    bool sauvegarde = false;
    cv::namedWindow("MonHDR");
    bool isGray = false;
    int typeHdr = 1;
    std::string path = "./etudeImages";
    //std::string path = "/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/etudeImages";
    
    afficheArg();
    if (argc >= 2) {
        typeHdr = std::atoi(argv[1]);
        if (argc >= 3) {
            filename = argv[2];
            sauvegarde = true;
            if(argc >= 4) {
                path = argv[3];
            }
        }
    } else {
       exit(-1);
    }
    
    //initialise la structure pour les données vidéo
    gData.init(false); // init dark
    
    //initialise la structure pour les données de calibrage
    dataCal.initCalibrage();
    
    cv::Mat image(gHeight,gWidth,CV_8UC3,{255,255,255});
    
    gData.img = image; 
    cv::imshow("MonHDR", image);
    cv::moveWindow("MonHDR", 0, 0);
    
    std::cout << " init " << std::endl;
    // init uvc data (webcam)
    tryUvc("init", uvc_init(&(gCxt), nullptr));
    
    std::cout << " find_device " << std::endl;
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 0, nullptr)); //cam portable
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 2057, nullptr)); //logitec
    tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 12314, nullptr)); //hercules
    //tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 19911, nullptr));
    
    std::cout << " uvc_open " << std::endl;
    tryUvc("open", uvc_open(gDev, &(gDevh)));
    
    std::cout << " get_ctrl " << std::endl;
    tryUvc("get_ctrl", uvc_get_stream_ctrl_format_size(gDevh, 
                &(gCtrl), UVC_FRAME_FORMAT_ANY, gWidth, gHeight, 30));

    std::cout << " set_ae_mode " << std::endl;
    tryUvc("set_ae_mode", uvc_set_ae_mode(gDevh, 1));
    
    std::cout << " set_exp_abs " << std::endl;
    //pour hercules
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gData.expMin));
    
    gFrame = uvc_allocate_frame(gWidth * gHeight * 3);
    if (not gFrame) {
        std::cerr << "unable to allocate frame!\n";
        exit(-1);
    }
    
    
    ChronoExp.restart();
    
    try { 
        
        
        cv::Mat response; //stockage de la reponse de calibrage 
        if (typeHdr != 0) {
           tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, 1)); 
           //appele la fonction de calibrage
           response = calibrageCam(typeHdr); // 0 mertens - 1 Debevec - 2 Robertson
           if (sauvegarde) {
               dataCal.saveImgs(path,"Calibrage_"+filename);
           }
        }
        
        std::cout << " start streaming vidéo" << std::endl;
        tryUvc("start_streaming", uvc_start_streaming(gDevh, 
            &(gCtrl), captureStd, nullptr , 0));
        
        cv::Mat imgHDR;
        cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
        cv::Ptr<cv::MergeRobertson> merge_robertson = cv::createMergeRobertson();
        cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
        
        cv::Mat imgBgr;
        cv::Mat imgGray;
        cv::Ptr<cv::TonemapDurand> tonemap = cv::createTonemapDurand(2.2f);
        cv::Mat dest;
    
        int cptCapture = 2;     
        int cptBrigthness =  0; //Pour le calcul auto de gamma
        imgIndex = 0;           //image traitée 
        float gamma = 2.2;
        float gammaSave;

        //bool save = true;
        bool afficheImages = false;
        
        tempsTestChrono.restart();
        
        while (cv::getWindowProperty("MonHDR", 0) != -1) {
            //gestion des touches
            
            switch (cv::waitKey(3) % 0x100) {
                case 27: // terminate when <esc> is hit
                    cv::destroyAllWindows();
                    std::cout << "fermeture" << std::endl;
                    break;
//                case 45: // gamma correction -0.1 <-> 
//                    tonemap->setGamma(tonemap->getGamma()-0.1f); 
//                    std::cout << "gamma " << tonemap->getGamma() << std::endl;
//                    break;
//                case 43:// gamma correction +0.1 <+>
//                    tonemap->setGamma(tonemap->getGamma()+0.1f);
//                    std::cout << "gamma " << tonemap->getGamma() << std::endl;
//                    break; 
//                case 56: // <8> contrast correction +0.1 <-> 
//                    tonemap->setContrast(tonemap->getContrast()+0.1f);
//                    std::cout << "contrast " << tonemap->getContrast() << std::endl;
//                    break;
//                case 50:// <2> contrast correction -0.1 <2>
//                    tonemap->setContrast(tonemap->getContrast()-0.1f);
//                    std::cout << "contrast " << tonemap->getContrast() << std::endl;
//                    break;
//                case 52: // <4> contrast correction +0.1 <4> 
//                    tonemap->setSaturation(tonemap->getSaturation()-0.1f);
//                    std::cout << "Saturation " << tonemap->getSaturation() << std::endl;
//                    break;
//                case 54:// <6> contrast correction -0.1 <6>
//                    tonemap->setSaturation(tonemap->getSaturation()+0.1f);
//                    std::cout << "Saturation " << tonemap->getSaturation() << std::endl;
//                    break;  
                case 53:// <5> inverse isGray <5>
                    isGray = !isGray;
                    break;  
                
                case 108:// <l> change luminosité <l>
                   { std::lock_guard<std::mutex> lock(gMutex); 
                     capture = false;
                     gData.init(!gData.brigth);
                     std::cout << "passage mode " << ((gData.brigth) ? "Bright" : "Dark") << std::endl;
                     std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                     capture = true;                     
                   }
                    break;  
                case 115:// <s> save images <s>
                   { std::lock_guard<std::mutex> lock(gMutex); 
                     capture = false;
                     
                     if (isGray) {
                         gData.saveHdr(imgGray,path,filename+"gray");
                     } else gData.saveHdr(imgBgr,path,filename+"Bgr");
                    
                     if (gData.brigth) {
                        gData.saveImgs(path,"video_Brigth_"+filename); 
                     } else gData.saveImgs(path,"video_Dark_"+filename);
                     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                     capture = true;                     
                   }
                    break;    
                
                    
                case 46:// <.> ferme les fenetres <.>
                    cv::destroyWindow("img0");
                    cv::destroyWindow("img1");
                    cv::destroyWindow("img2");
                    cv::destroyWindow("img3");
                    
                    cv::destroyWindow("img0_1");
                    cv::destroyWindow("img1_300");
                    cv::destroyWindow("img2_20");
                    cv::destroyWindow("img3_500");
                    cv::destroyWindow("img4_100");
                    cv::destroyWindow("img5_1000");
                    break;  
                case 48: // <0> affiche immages 
                    afficheImages = !afficheImages;
                    break;
                default:;
            }
             
            std::lock_guard<std::mutex> lock(gMutex);  
            if (imgIndex != cptCapture) {
                tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gData.exp[imgIndex])); 
               // cptCapture = imgIndex;
  
               // if (cptCapture != gData.nbImg-1){
                    std::this_thread::sleep_for(std::chrono::milliseconds((gData.exp[imgIndex]*(1/10)) +32));
                    capture = true;
               // }
                cptCapture = imgIndex;
            } 
            
            if (cptCapture == gData.nbImg-1) { // traite image
                cptBrigthness++;
                if (typeHdr == 1) { 
                    merge_debevec->process(gData.pictures, imgHDR, gData.times, response);
                    tonemap->process(imgHDR, imgBgr);
                } else if (typeHdr == 2 ) {
                            merge_robertson->process(gData.pictures, imgHDR, gData.times, response);
                            tonemap->process(imgHDR, imgBgr);
                        } else { // mertens
                               //merge_mertens->process(gData.pictures, imgHDR, gData.times, response); 
                               merge_mertens->process(gData.pictures, imgBgr);
                               
                        }
                
                if (isGray) {
                    col2Gray(imgBgr, imgGray);
                    cv::imshow("MonHDR", imgGray);
                    std::cout << cv::mean(imgGray) << std::endl;        
                } else {
                    if(cptBrigthness > 9 && typeHdr != 0) {
                        cv::extractChannel(imgBgr,imgGray,1);
                        cv::Scalar moy = cv::mean(imgGray);
                        gammaSave = gamma;
                        gamma = 0.5f/(moy[0]/tonemap->getGamma());
                        if(isnan(gamma)){//|| (std::abs(gamma - 0.5f) < 0.02f) ) {
                           gamma = gammaSave;  
                        }                   
                        tonemap->setGamma(gamma);
                        cptBrigthness = 0;
                    }
                    
                    cv::imshow("MonHDR", imgBgr);
                    if (afficheImages) {
                        cv::imshow("img0",gData.pictures[0]);
                        cv::imshow("img1",gData.pictures[1]);
                        cv::imshow("img2",gData.pictures[2]);
                        cv::imshow("img3",gData.pictures[3]);
                    }
                }
                capture = true;
            }
        }
    }
    catch (cv::Exception&) {
    } 
    return 0;
}

/** @brief call uvc function and check return code
 * @param msg pointeur on error message
 * @param res Methode appelée qui retourne un code retour
 */ 
void tryUvc(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        exit(-1);
    }
}

/** @brief call uvc function and check return code
 * @param msg pointeur on error message
 * @param res Methode appelée qui retourne un code retour
 * @return code de retour
 */ 
uvc_error_t tryUvcRet(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        //exit(-1);
    }
    return res;
}

// capture pour vidéo
void captureStd(uvc_frame_t * frame, void *) {
    //double elapse = ChronoExp.elapsedRunning();
    //std::cout << ".";
    if (capture ) {
       if (gData.nbframe > 0) {
            tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));

            //copy frame to Vector Pictures
            cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
            std::lock_guard<std::mutex> lock(gMutex);

            mat.copyTo(gData.pictures[imgIndex]);
            // change exposure time for next capture    
            imgIndex = (imgIndex + 1) % gData.nbImg;

            capture = false;
            gData.nbframe =0;
       } else {
            gData.nbframe++;
       }
       
    }
    //ChronoExp.restart();
}

// capture pour calibrage
void captureCal(uvc_frame_t * frame, void *) {
    double elapse = ChronoExp.elapsedRunning();
    std::cout << ".";
    if (capture 
            && elapse > dataCal.bornes[imgIndex].bInf
            &&  elapse < dataCal.bornes[imgIndex].bSup) {
      
       tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));
       
       //copy frame to Vector Pictures
       cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
       std::lock_guard<std::mutex> lock(gMutex);

       mat.copyTo(dataCal.pictures[imgIndex]);
       //std::cout << imgIndex << " " << tempsTestChrono.elapsedAndRestart() << std::endl;
       
       // change exposure time for next capture    
       imgIndex = (imgIndex + 1) % dataCal.nbImg;
   
       capture = false;
       
    }
    ChronoExp.restart();
}

//Calibrage de la camera
cv::Mat calibrageCam(int type) {
    std::cout << " start streaming calibrage" << std::endl;
    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
                &(gCtrl), captureCal, nullptr , 0));
    
    int idx = 0;
       
    capture = true;

    int test = 0 ;
    Chrono elapseTotal;
    elapseTotal.restart();
    if (type != 0) {
        // recuperation images pour Calibrage;
        std::cout << "Calibrage camera ....." << std::endl;
        
        uint32_t timest;
        tryUvc("set_exp_abs", uvc_get_exposure_abs(gDevh, &(timest), UVC_GET_MIN));
        std::cout << "Min time ....." << timest << std::endl;
        tryUvc("set_exp_abs", uvc_get_exposure_abs(gDevh, &(timest), UVC_GET_MAX));
        std::cout << "Max time ....." << timest << std::endl;
        tryUvc("set_exp_abs", uvc_get_exposure_abs(gDevh, &(timest), UVC_GET_CUR));
        std::cout << "cur time ....." << timest << std::endl;
        
        while (cv::getWindowProperty("MonHDR", 0) != -1 && test < 6) {
            //gestion des touches
             if (cv::waitKey(3) % 0x100 == 27) {
                    cv::destroyAllWindows();
                    std::cout << "fermeture " << std::endl;
             }
             // prevent concurrency (webcam capture)
             std::lock_guard<std::mutex> lock(gMutex);             
             if (imgIndex != idx) {
                tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, dataCal.exp[imgIndex]));
                idx = imgIndex;
                std::this_thread::sleep_for(std::chrono::milliseconds(dataCal.exp[imgIndex]/10+30));
                //std::cout << "w" << dataCal.exp[imgIndex]/10+30;
                capture = true;
                test++;
             }
        }

        //std::cout << "temps " << elapseTotal.elapsedRunning() << std::endl;
        elapseTotal.stop();

        capture = false;
        
        //affiche les images de calibrage pour verification
        cv::imshow("img0_1", dataCal.pictures[0]);
        cv::imshow("img1_300", dataCal.pictures[1]);
        cv::imshow("img2_20", dataCal.pictures[2]);
        cv::imshow("img3_500", dataCal.pictures[3]);
        cv::imshow("img4_100", dataCal.pictures[4]);
        cv::imshow("img5_1000", dataCal.pictures[5]);

        elapseTotal.restart();
    }
    
    
    //tryUvc("stop_streaming", uvc_stop_streaming(gDevh));
    uvc_stop_streaming(gDevh);
    if (type == 1) { //debevec     
        cv::Mat responseDebevec;
        cv::Ptr<cv::CalibrateDebevec> calibrateD = cv::createCalibrateDebevec();
        calibrateD->process(dataCal.pictures, responseDebevec, dataCal.times);
        std::cout << "Calibrage Debevec" << elapseTotal.elapsedRunning() << std::endl;
        elapseTotal.stop();           
        /////Fin callibrage
        return responseDebevec;
    } else { // robertson
                cv::Mat responseRobertson;
                cv::Ptr<cv::CalibrateRobertson> calibrateR = cv::createCalibrateRobertson();
                calibrateR->process(dataCal.pictures, responseRobertson, dataCal.times);
                std::cout << "Calibrage Robertson" << elapseTotal.elapsedRunning() << std::endl;
                elapseTotal.stop();           
                /////Fin callibrage
                return responseRobertson;
            } 
    
}

void col2Gray(const cv::Mat & imgCo, cv::Mat &imgGray ) {
    cv::cvtColor( imgCo, imgGray, CV_BGR2GRAY );
}



