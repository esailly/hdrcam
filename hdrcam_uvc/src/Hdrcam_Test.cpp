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
 * File:   Hdrcam_Test.cpp
 * Author: esailly
 *
 * Created on 18 mai 2017, 11:19
 */

#include <cstdlib>
#include <opencv2/photo.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include "Chrono.hpp"

/* Déclarations des fonctions */
void loadExposureSeq(std::string, std::string, std::vector<cv::Mat>&, std::vector<float>&, int &);
void saveResponse(std::string, cv::Mat &);
/*
 * 
 */
int main(int argc, char** argv) {
    
    std::cout << "Usage: " << argv[0] << " <filename> <namepath>" << std::endl;
    std::cout << "Le répertoir ./etudeImages/ doit exister." << std::endl;
    std::cout << "Lancer .HdrCam_Video.out et sauvegarder e répertoir ./etudeImages/ doit exister." << std::endl;
    
    std::string namepath = argv[2];
    std::string namefile = argv[1];
    bool automatique = false;
    //if (argc > 2 ) automatique = true;
    std::vector<cv::Mat> images;
    std::vector<float> times;
    Chrono chrono;
    int time;
    int time2 = 0;
    //int moyenne = 0;
    int nbImages = 0;
    
    //loadExposureSeq(namepath, namefile, images, times, moyenne);
    loadExposureSeq(namepath, namefile, images, times, nbImages);
    std::ofstream myfile ("./etudeImages/recap.txt", std::ios::out|std::ios::app);//|std::ios::out);
    //myfile << namefile << " " << moyenne;
    myfile << namefile << " " << nbImages;
    
    //// Test Debevec
    chrono.restart();
    cv::Mat responseDebevec;
    cv::Ptr<cv::CalibrateDebevec> calibrateD = cv::createCalibrateDebevec();
    calibrateD->process(images, responseDebevec, times);
    time = chrono.elapsedRunning();
    std::cout << " Calibrate Debevec : " << time << std::endl; 
    myfile << " " << time;
    time2 = time;
    
    chrono.restart();
    cv::Mat hdrDeb;
    cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
    merge_debevec->process(images, hdrDeb, times, responseDebevec);
    time = chrono.elapsedRunning();
    std::cout << "merge Debevec : " << time << std::endl; 
    myfile << " " << time;
    time2 += time;
    
    chrono.restart();
    cv::Mat deb;
    cv::Ptr<cv::TonemapDurand> tonemap = cv::createTonemapDurand(2.2f);
    tonemap->process(hdrDeb, deb);
    time = chrono.elapsedRunning();
    std::cout << "Tone Map Durand : " << time << std::endl; 
    time2 += time;
    myfile << " " << time << " " << time2;
    
    
    //// Test Robertson
    chrono.restart();
    cv::Mat responseRobertson;
    cv::Ptr<cv::CalibrateRobertson> calibrateR = cv::createCalibrateRobertson();
    calibrateR->process(images, responseRobertson, times);
    time = chrono.elapsedRunning();
    std::cout << "Calibrate Robertson : " << time << std::endl; 
    myfile << " " << time;
    time2 = time;
    
    chrono.restart();
    cv::Mat hdrRob;
    cv::Ptr<cv::MergeRobertson> merge_Robertson = cv::createMergeRobertson();
    merge_debevec->process(images, hdrRob, times, responseRobertson);
    time = chrono.elapsedRunning();
    std::cout << "merge Robertson : " << time << std::endl; 
    time2 += time;
    myfile << " " << time;
    
    chrono.restart();
    cv::Mat rob;
    cv::Ptr<cv::TonemapDurand> tonemap2 = cv::createTonemapDurand(2.2f);
    tonemap2->process(hdrRob, rob);
    time = chrono.elapsedRunning();
    std::cout << "Tone Map Durand : " << time << std::endl; 
    time2 += time;
    myfile << " " << time << " " << time2;
    
    chrono.restart();
    cv::Mat fusion;
    cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
    merge_mertens->process(images, fusion);
    time = chrono.elapsedRunning();
    std::cout << "Merge merten (fusion) : " << time << std::endl; 
    myfile << " " << time << std::endl;
    
    
    
    
    cv::imshow("fusion", fusion);
    cv::imshow("Debevec", deb);
    cv::imshow("Robertson", rob);
    
    
    cv::imshow("hdrDeb", hdrDeb);
    cv::imshow("hdrRob", hdrRob);
    
    saveResponse(namepath+"/"+namefile+"CalDebevec.csv",responseDebevec);
    saveResponse(namepath+"/"+namefile+"CalRotertson.csv",responseRobertson);
    
    cv::imwrite(namepath+"/"+namefile+"_fusion.png", fusion);
    cv::imwrite(namepath+"/"+namefile+"_Deb.png", deb);
    cv::imwrite(namepath+"/"+namefile+"_Rob.png", rob);
    
    cv::imwrite(namepath+"/"+namefile+"_HdrDeb.hdr", hdrDeb);
    cv::imwrite(namepath+"/"+namefile+"_HdrRob.hdr", hdrRob);
    
    
//    imwrite("fusion.png", fusion * 255);
//    imwrite("ldr.png", ldr * 255);
//    imwrite("hdr.hdr", hdr);
    
    while (!automatique && true) {

        // handle user events
        int key = cv::waitKey(30);
        if (key != 255) std::cout << key << " " ;
        if (key % 0x100 == 27) 
            break;
    }   
    
    
    return 0;
}

void loadExposureSeq(std::string path, std::string file, std::vector<cv::Mat>& images, std::vector<float>& times, int & moyenne)
{
    path = path + std::string("/");
    
    std::ifstream list_file((path + file).c_str());
    std::string name;
    float val;
    //int moyenne;
    list_file >>  name >> moyenne;
    //list_file >>  name >> nbimages;
    while(list_file >> name >> val) {
        cv::Mat img = cv::imread(path + name);
        images.push_back(img);
        times.push_back(val);
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
    }   
   myfile.close();
}