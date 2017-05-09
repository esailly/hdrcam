/* 
 * File:   PictureHdr.hpp
 * Author: esailly
 *
 * Created on 13 avril 2017, 13:16
 */

#ifndef PICTUREHDR_HPP
#define PICTUREHDR_HPP

#include <opencv2/opencv.hpp>
#include <vector>

class PictureHdr {
public:
    PictureHdr();
    PictureHdr(const PictureHdr& orig);
    PictureHdr(int nbPictures);
    virtual ~PictureHdr();
    
    std::vector<cv::Mat> & pictures() {
        return & pictures;
    }

    std::vector<float> & times() {
        return & times;
    }
    
    std::vector<int> & exp() {
        return & exp;
    }
    
    void setTimes(int min, int max); 
    void setExp(int _interval);
    
    void hdrDebevec(cv::Mat & hdrPicture);
    void hdrRobertson(cv::Mat & hdrPicture);
    void hdrmertens(cv::Mat & hdrPicture);
    
private:
    
   int nbPictures;
   float minExp;
   float maxExp;
   std::vector<cv::Mat> pictures;
   std::vector<float> times; 
   std::vector<int> exp;
   
   cv::Mat img;
   
};

#endif /* PICTUREHDR_HPP */

