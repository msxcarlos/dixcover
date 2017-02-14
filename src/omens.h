#ifndef OMENS_H
#define OMENS_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/text.hpp"

#include <QImage>
#include <QPixmap>


class Omens
{
public:
    Omens();
    cv::Mat textDetection(const cv::Mat &img);
    QImage textDetection(const QImage &img);
    QPixmap textDetection(const QPixmap *pixMap);
    QString getWordsDetection();
    //inline std::vector<std::pair<cv::Point2i, std::string> detectedWords() {return words_detection;}

private:
    bool  isRepetitive(const std::string& s);
    //Draw ER's in an image via floodFill
    void  er_draw(std::vector<cv::Mat> &channels, std::vector<std::vector<cv::text::ERStat> > &regions, std::vector<cv::Vec2i> group, cv::Mat& segmentation);
    //Detected words
    //std::vector<std::pair<cv::Point2i, std::string> words_detection;
    std::vector<std::string> words_detection;


};



#endif // OMENS_H
