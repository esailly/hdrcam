/* 
 * File:   PictureHdr.cpp
 * Author: esailly
 * 
 * Created on 13 avril 2017, 13:16
 */

#include "PictureHdr.hpp"

PictureHdr::PictureHdr() {
}

PictureHdr::PictureHdr(const PictureHdr& orig) {
}

PictureHdr::PictureHdr(int nbPictures) {
    this->nbPictures = nbPictures;
    pictures.resize(nbPictures);
    times.resize(nbPictures);
    setTimes(30,60);
}

PictureHdr::~PictureHdr() {
}

void PictureHdr::setTimes(int min, int max, int _interval) {
    minExp = min;
    maxExp = max;
    int interval = (max - min)/(nbPictures-1); 
    times[0] = (float) min / (float) 1000;
    int i=1;
    while (i < times.size()-1) {
       times[i] = (min + (interval*i)) / (float) 1000;
       i++;
    }
    times[i] = (float) max / (float) 1000;
}

void PictureHdr::setExp(int _interval) {
    for (int i = 0; i < nbPictures-2; i++) {
        exp[i] = minExp + (_interval*1);
    }
    exp[nbPictures-1] = maxExp;
}


void PictureHdr::hdrDebevec(cv::Mat & hdrPicture) {
    cv::Mat reponseDebevec;
    cv::Ptr<cv::CalibrateDebevec> calibrateDebevec = cv::createCalibrateDebevec();
    calibrateDebevec->process(pictures, reponseDebevec, times);
    
    //to do changer avecc path
    saveResponse("/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/CalDebevec.csv",reponseDebevec);
    
    cv::Mat aPicture;
    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
    merge_debevec->process(images, aPicture, times, reponseDebevec);
    
    cv::Ptr<cv::TonemapDurand> tonemap1 = cv::createTonemapDurand(gamma=2.2);
    tonemap1->process(aPicture, hdrPicture);
}

cv::Mat PictureHdr::hdrRobertson(cv::Mat & hdrPicture) {
    cv::Mat reponseRobertson;
    cv::Ptr<cv::CalibrateRobertson> calibrateRobertson = cv::createCalibrateRobertson();
    calibrateRobertson->process(pictures, reponseRobertson, times);
    
    //to do changer avecc path
    saveResponse("/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/CalRobertson.csv",reponseRobertson);
    
    cv::Mat aPicture;
    cv::Ptr<cv::MergeRobertson> merge_Robertson = cv::createMergeRobertson();
    merge_Robertson->process(images, aPicture, times, reponseRobertson);
    
    cv::Ptr<cv::TonemapDurand> tonemap1 = cv::createTonemapDurand(gamma=2.2);
    tonemap1->process(aPicture, hdrPicture);
}

cv::Mat PictureHdr::hdrmertens(cv::Mat & hdrPicture) {
    //cv::Mat fusion;
    cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
    merge_mertens->process(images, hdrPicture);
}