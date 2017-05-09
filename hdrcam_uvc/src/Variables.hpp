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

//#include "PictureHdr.hpp"


/** @brief DataStructure for Callbacks functions
 * 
 */
struct Data {
    cv::Mat img; 
    int expMin = 300;
    int expMax = 600;
    int interval = 300;
    int nbImg  = 2;
    uint32_t seq =0;
    int nbframe =0;
    cv::Mat reponse;
    //int viewCalcImg = 0; 
    
    std::vector<cv::Mat> pictures;
    std::vector<float> times; 
    std::vector<int> exp;
    std::vector<int> mAvg;
   
    void init() {
        
        pictures.resize(2);
           cv::Mat image(480,640,CV_8UC3,{255,255,255});
           pictures[0]=image;
           cv::Mat image2(480,640,CV_8UC3,{255,255,255});
           pictures[1]=image2;
        times.resize(2);
        exp.resize(2);
        mAvg.resize(2);
        
        setNbImg(2);
//        std::cout << "Init : expMin " << expMin;
//        for (unsigned i = 0; i < exp.size(); i++) {
//            std::cout << "exp[" << i <<"]" << exp[i];
//        }
//        std::cout << std::endl;
          
    }
    
    void init(int _min, int _max, int _nbImg) {
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
        
        setInterval();
        setExp();
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
            if (_min < 30) {
                expMin = 30;
            }
        } else {
            if (expMax - (getNbInterval()*30) < 30) {
               expMin = 30;
               expMax = expMin + (getNbInterval()*30);
            } else expMin = expMax - (getNbInterval()*30);
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
            if(expMin + (getNbInterval()*30) > 10000) {
               expMax = 10000; 
               expMin = expMax - (getNbInterval()*30);
            } else expMax = expMin  + (getNbInterval()*30);
        }
        setInterval();
        setExp();
        setTimes();
    }
    
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
             default: std::cout << "default\n"; // no error
             break;
        }
    }
    
    void setNbImg(int _nb) {
        
        if (_nb < 2 ) {
           nbImg = 2;  
        } else nbImg = _nb;  
        setInterval();
        
        if (interval < 30 ) {
           expMax= expMin + (getNbInterval()*30); 
           setInterval();
        }
        
        pictures.resize(_nb);
           
           for (int i = 0; i < _nb; i++) {
              cv::Mat image(480,640,CV_8UC3,{255,255,255}); 
              pictures[i] = img = image.clone();
            }

//        
//           cv::Mat image(480,640,CV_8UC3,{255,255,255});
//           
//           
//           for (auto img : pictures) {
//               img = image.clone();
//        }
        //   pictures[0]=image.clone();
        
        times.resize(_nb);
        exp.resize(_nb);
        mAvg.resize(_nb);
        setExp();
        setTimes();
        setMAvg();        
    } 
    
    void setExp() {
        for (int i = 0; i < nbImg-1; i++) {
            exp[i] = expMin + (interval*i);
        }
        exp[nbImg-1] = expMax;
        
    }
    
    void setTimes() {
        //interval = (expMax - expMin)/(nbImg-1); 
        times[0] = (float) expMin / (float) 10000; //dark 100/10000 = 0.01
        unsigned i=1;
        while (i < times.size()-1) {
           times[i] = ((float) expMin + ((float) interval*i)) / (float) 10000;
           i++;
        }
        times[i] = (float) expMax / (float) 10000;
        std::cout << "Times : {";
        for (auto elem : times) {
            std::cout << elem << ",";
        }
        std::cout << "}" << std::endl;
    }

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
    }
};

#endif /* VARIABLES_HPP */

