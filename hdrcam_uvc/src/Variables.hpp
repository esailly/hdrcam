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
 * File:   Variables.hpp
 * Author: esailly
 *
 * Created on 27 avril 2017, 14:54
 */

#ifndef VARIABLES_HPP
#define VARIABLES_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>

struct Borne {
    int bInf;
    int bSup;
};

/** @brief DataStructure for Callbacks functions
 * 
 */
struct Data {
    cv::Mat img; 
    bool brigth = false;
    int expMin = 30;
    int expMax = 60;
    int interval = 30;
    int nbImg  = 2;
    uint32_t seq =0;
    int nbframe =0;
    cv::Mat reponse;
    
    std::vector<cv::Mat> pictures;
    std::vector<float> times; 
    std::vector<int> exp;
    std::vector<int> mAvg;
    std::vector<Borne> bornes;
   
    void init() {
        
        pictures.resize(2);
           cv::Mat image(480,640,CV_8UC3,{255,255,255});
           pictures[0]=image;
           cv::Mat image2(480,640,CV_8UC3,{255,255,255});
           pictures[1]=image2;
        times.resize(2);
        exp.resize(2);
        mAvg.resize(2);
        bornes.resize(2);        
        setNbImg(2);
        
    }
    
    void init(bool _brigth) {
        brigth = _brigth;
        if (brigth) {
           std::cout << "true " <<  std::endl; 
           init(10, 333, 4, true); 
        } else {
           std::cout << "false " <<  std::endl; 
           init(100, 700, 4, true); 
        }
    }
    
    void init(int _min, int _max, int _nbImg, bool automatique = false) {
        expMin = _min;
        expMax = _max;
        nbImg  = _nbImg; 
        
        pictures.resize(_nbImg);
        for (unsigned i = 0; i < pictures.size(); i++) {
            cv::Mat image(480,640,CV_8UC3,{255,255,255});
            pictures[i] = image.clone();
        }
        
            times.resize(_nbImg);
            exp.resize(_nbImg);
            mAvg.resize(_nbImg); 
            bornes.resize(_nbImg);
            
        if (automatique) {
            setExpFixe();
        } else {
            setInterval(); 
            setExp();
        }
        
        setBornes();
        setTimes();       
        
        std::cout << "expMin   : "<< expMin << std::endl;
        std::cout << "expMax   : "<< expMax << std::endl;
        std::cout << "nbImg    : "<< nbImg << std::endl;
        std::cout << "nbinter  : "<< getNbInterval() << std::endl;
        std::cout << "interval : "<< interval << std::endl;
        std::cout << "Exp      : {";
        for (auto elem : exp) {
            std::cout << elem << ",";
        }
        std::cout << "}" << std::endl;
        std::cout << "Times    : {";
        for (auto elem : times) {
            std::cout << elem << ",";
        }
        std::cout << "}" << std::endl;
   
    }
    
    void initCalibrage() {
        expMin = 1;
        expMax = 1000;
        nbImg  = 6;
        
        pictures.resize(6);
        for (unsigned i = 0; i < pictures.size(); i++) {
            cv::Mat image(480,640,CV_8UC3,{255,255,255});
            pictures[i] = image.clone();
        }
        
        times.resize(6);
        //abritrairement 6 temps de pose
        exp.resize(6);
        exp[0] = 10;   // 1/1000"
        exp[1] = 333; // 1/30"
        exp[2] = 20;  // 20/10000"
        exp[3] = 500; // 1/20"
        exp[4] = 100; // 1/100"
        exp[5] = 1000; //1/10"

        mAvg.resize(6);
        
        bornes.resize(6);
        for (unsigned i = 0; i < bornes.size(); i++) {
            bornes[i].bInf = (exp[i] < 10 ) ?  exp[i] : exp[i]/10 ;
            if (i < bornes.size()-1) { //cas std
               bornes[i].bSup = (exp[i] < 10 ) ?  32 : exp[i]/10 + 32 ;
            } else  { //Dernier
                bornes[i].bSup = 1000;
            }
        }
        
        for (unsigned i = 0; i < exp.size(); i++) {
            times[i] = (float) exp[i] / (float) 10000;
        }
        std::cout << "calibrage " << std::endl;
        printTimes();
        
    }
    
    void setInterval() {
        interval = (expMax-expMin)/getNbInterval();
    }
    
    int getNbInterval() {
        if (nbImg == 2 ) {return 1;} else return nbImg -1;
    }
    
    void setExpMin(int _min) {
        if (_min < expMax) {
            //ok pour changement
            expMin = _min;
            if (_min < 1) {
                expMin = 1;
            }
        } else {
            if (expMax - (getNbInterval()) < 1) {
               expMin = 1;
               expMax = expMin + (getNbInterval());
            } else expMin = expMax - (getNbInterval());
        }
        
        setInterval();
        setExp();
        setTimes();
    } 
    
    void setExpMax(int _max) { 
        if (_max > expMin) {
            //ok pour changement
            expMax = _max;
        }else {
            if(expMin + (getNbInterval()) > 10000) {
               expMax = 10000; 
               expMin = expMax - (getNbInterval());
            } else expMax = expMin  + (getNbInterval());
        }
        setInterval();
        setExp();
        setTimes();
    }
    
    //
    void setMAvg() {
        switch (nbImg){
            case 2: {  
                mAvg[0] = 64;
                mAvg[1] = 189;
               break;
            } 
            case 3: {
                mAvg[0] = 64;
                mAvg[1] = 125;
                mAvg[2] = 189;
                break;
            }
            case 4: {
                mAvg[0] = 64;
                mAvg[1] = 105;
                mAvg[2] = 146;
                mAvg[3] = 189;
                break;
            }
            case 5: {
                mAvg[0] = 64;
                mAvg[1] = 95;
                mAvg[2] = 125;
                mAvg[3] = 156;
                mAvg[4] = 189;
                break;
            }
            case 6: {
                mAvg[0] = 64;
                mAvg[1] = 95;
                mAvg[2] = 125;
                mAvg[3] = 156;
                mAvg[4] = 189;
                mAvg[5] = 189;
                break;
            }
             default: std::cout << "default\n"; // no error
             break;
        }
    }
    
    void setNbImg(int _nb) {
        
        if (_nb < 2 ) {
           nbImg = 2;  
        } else nbImg = _nb;  
        setInterval();
        
        if (interval < 10 ) {
           expMax= expMin + (getNbInterval()*10); 
           setInterval();
        }
        
        pictures.resize(_nb);
           
           for (int i = 0; i < _nb; i++) {
              cv::Mat image(480,640,CV_8UC3,{255,255,255}); 
              pictures[i] = img = image.clone();
            }

        times.resize(_nb);
        exp.resize(_nb);
        mAvg.resize(_nb);
        bornes.resize(_nb);
        setExp();
        setBornes();
        setTimes();
        setMAvg();        
    } 
    
    void setExp() {
     
        for (int i = 0; i < nbImg; i++) {
            exp[i] = expMin + (interval*i);
        }
        exp[nbImg-1] = expMax;
        
    }
    
    void setExpFixe() {
        
        if (brigth) {
//            exp[0] = 10;
//            exp[1] = 333;
//            exp[2] = 33;
//            exp[3] = 200;
            exp[0] = 10;
            exp[1] = 33;
            exp[2] = 200;
            exp[3] = 333;
            expMin = 10;
            expMax = 333;
        } else {
            exp[0] = 100;
            exp[1] = 500;
            exp[2] = 200;
            exp[3] = 1000;
            expMin = 10;
            expMax = 700;
        }
        
    }
    
    void setTimes() {        
        for (unsigned i = 0; i < times.size(); i++) {
            times[i] = ((float) exp[i] / (float) 10000.);
        }          
        
        std::cout << "Times : {";
        for (auto elem : times) {
            std::cout << elem << ",";
        }
        std::cout << "}" << std::endl;
    }
    
    void setBornes() {
         for (unsigned i = 0; i < bornes.size(); i++) {
            bornes[i].bInf = (exp[i] < 10 ) ?  exp[i] : exp[i]/10 ;
            if (i < bornes.size()-1) { //cas std
               bornes[i].bSup = (exp[i] < 20 ) ?  32 : exp[i]/10 + 32 ;
            } else  { //Dernier
                bornes[i].bSup = 1000;
            }
        }
        
        std::cout << "Bornes : {";
        for (auto elem : bornes) {
            std::cout << "[" << elem.bInf << "," << elem.bSup << "],"; 
        }
        std::cout << "}" << std::endl;
    }
    
    //verifier utilité
    int getImageAvg(int idx) {
        cv::Mat gray_image;
        cv::cvtColor( pictures[idx], gray_image, CV_BGR2GRAY );
        
        cv::Scalar val; 
        val = cv::mean(gray_image);
        //std::cout << val << std::endl;
        return val[0];
    }
    
    void printTimes() {
        std::cout << "Times : {";
        for (auto elem : times) {
            std::cout << elem << ",";
        }
        std::cout << "}" << std::endl;
        
        std::cout << "Exp : {";
        for (auto elem : exp) {
            std::cout << elem << ",";
        }
        std::cout << "}" << std::endl;
        
        std::cout << "Bornes : {";
        for (auto elem : bornes) {
            std::cout << "[" << elem.bInf << "," << elem.bSup << "],"; 
        }
        std::cout << "}" << std::endl;
    }
    
    //sauvegarde des images de calcul
    void saveImgs(std::string path, std::string filename) {
        std::string file = path + "/" +filename;
        std::cout << file <<std::endl;
        std::ofstream myfile (file + "_List.txt", std::ios::out);

        std::vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(0);
        
        myfile << "Detail nbImages :" << nbImg << std::endl;
        for (unsigned i = 0; i < pictures.size(); i++) {
            cv::imwrite( file + "(" + std::to_string(i) + ").png", 
                         pictures[i],
                         compression_params);
            myfile << (filename + "(" + std::to_string(i)+").png ") 
                   << times[i] << " " << " " << std::endl;
        }

        myfile.close();
    }
    //sauvegarde de l'image fournie
    void saveHdr(cv::Mat img, std::string path, std::string filename) {
        std::string file = path + "/" +filename;
       
        std::vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(0);
        
        cv::imwrite( file + "hdr.png", 
                         img * 255,
                         compression_params);
    }
    
};

#endif /* VARIABLES_HPP */

