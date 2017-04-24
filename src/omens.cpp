#include "omens.h"
#include "cvmatandqimage.h"

#include <boost/filesystem.hpp>

using namespace cv;
using namespace std;
using namespace cv::text;
using namespace QtOcv;
using namespace boost::filesystem;

bool compare_rect(const cv::Rect & a, const cv::Rect &b);
bool lbp_isComplex(const string & path_tempImage);

Omens::Omens() {
}

Mat Omens::textDetection(const Mat &image,bool lbp_flag) {

    imwrite("../resources/tempImage.jpg",image);
    path absolutePath_tempImage = absolute("../resources/tempImage.jpg");
    bool isComplex = lbp_isComplex(absolutePath_tempImage.string());
    remove(absolutePath_tempImage.string());

    cout << "IMG_W=" << image.cols << endl;
    cout << "IMG_H=" << image.rows << endl;

    int grid_dim= 3; // Dimension of the grid to be considered, nxn blocks
    // The grid will consider two dimensions (x,y)
    vector<Rect> x_blocks;
    vector<Rect> y_blocks;
    vector< vector<Rect> > blocks;
    for (int i =0 ; i < grid_dim ; i++)
    {
        x_blocks.push_back(Rect(i*image.cols/grid_dim,0,image.cols/grid_dim,image.rows));
        y_blocks.push_back(Rect(0,i*image.rows/grid_dim,image.cols,image.rows/grid_dim));
    }
    blocks.push_back(x_blocks);
    blocks.push_back(y_blocks);
//    for (int i=0; i<grid_dim;i++)
//    {
//        for(int j=0;j<grid_dim;j++)
//        {
//            blocks.push_back(Rect(j*image.cols/grid_dim,i*image.cols/grid_dim,image.cols/3,image.rows/3));
//        }
//    }

    double t_d = (double)getTickCount();
    double t_r;

    /*Text Detection*/
    Mat out_img_detection;

    if(isComplex && lbp_flag)
    {
        cout << "COMPLEX IMAGE DETECTED"<< endl;

        // Extract channels to be processed individually
        vector<Mat> channels;

        Mat grey;
        cvtColor(image,grey,COLOR_RGB2GRAY);

        // Notice here we are only using grey channel, see textdetection.cpp for example with more channels
        channels.push_back(grey);
        channels.push_back(255-grey);

        /////// KAI
            computeNMChannels(image, channels);

            int cn = (int)channels.size();
            // Append negative channels to detect ER- (bright regions over dark background)
            for (int c = 0; c < cn-1; c++)
                channels.push_back(255-channels[c]);
        /////// KAI

        t_d = (double)getTickCount();

        // Create ERFilter objects with the 1st and 2nd stage default classifiers

        path absolutePath_trained_classifierNM1 = absolute("../resources/trained_classifierNM1.xml");
        path absolutePath_trained_classifierNM2 = absolute("../resources/trained_classifierNM2.xml");

        Ptr<ERFilter> er_filter1 = createERFilterNM1(loadClassifierNM1(absolutePath_trained_classifierNM1.string()),8,0.00015f,0.13f,0.2f,true,0.1f);
        Ptr<ERFilter> er_filter2 = createERFilterNM2(loadClassifierNM2(absolutePath_trained_classifierNM2.string()),0.5);

        vector<vector<ERStat> > regions(channels.size());
        // Apply the default cascade classifier to each independent channel (could be done in parallel)
        for (int c=0; c<(int)channels.size(); c++)
        {
            er_filter1->run(channels[c], regions[c]);
            er_filter2->run(channels[c], regions[c]);
        }
        cout << "TIME_REGION_DETECTION = " << ((double)getTickCount() - t_d)*1000/getTickFrequency() << endl;

        Mat out_img_decomposition= Mat::zeros(image.rows+2, image.cols+2, CV_8UC1);
        vector<Vec2i> tmp_group;
        for (int i=0; i<(int)regions.size(); i++)
        {
            for (int j=0; j<(int)regions[i].size();j++)
            {
                tmp_group.push_back(Vec2i(i,j));
            }
            Mat tmp= Mat::zeros(image.rows+2, image.cols+2, CV_8UC1);
            er_draw(channels, regions, tmp_group, tmp);
            if (i > 0)
                tmp = tmp / 2;
            out_img_decomposition = out_img_decomposition | tmp;
            tmp_group.clear();
        }

        double t_g = (double)getTickCount();
        // Detect character groups
        vector< vector<Vec2i> > nm_region_groups;
        vector<Rect> nm_boxes;
        erGrouping(image, channels, regions, nm_region_groups, nm_boxes,ERGROUPING_ORIENTATION_HORIZ);
        cout << "TIME_GROUPING = " << ((double)getTickCount() - t_g)*1000/getTickFrequency() << endl;

        /*Text Recognition (OCR)*/

        t_r = (double)getTickCount();
        Ptr<OCRTesseract> ocr = OCRTesseract::create(NULL, "spa");
        //Ptr<OCRTesseract> ocr = OCRTesseract::create();

        cout << "TIME_OCR_INITIALIZATION = " << ((double)getTickCount() - t_r)*1000/getTickFrequency() << endl;
        string output;

        Mat out_img;
        Mat out_img_segmentation = Mat::zeros(image.rows+2, image.cols+2, CV_8UC1);
        image.copyTo(out_img);
        image.copyTo(out_img_detection);
        float scale_img  = 600.f/image.rows;
        float scale_font = (float)(2-scale_img)/1.4f;


        t_r = (double)getTickCount();

        for (int i=0; i<(int)nm_boxes.size(); i++)
        {

            rectangle(out_img_detection, nm_boxes[i].tl(), nm_boxes[i].br(), Scalar(0,255,255), 3);

            Mat group_img = Mat::zeros(image.rows+2, image.cols+2, CV_8UC1);
            er_draw(channels, regions, nm_region_groups[i], group_img);
            Mat group_segmentation;
            group_img.copyTo(group_segmentation);
            //image(nm_boxes[i]).copyTo(group_img);
            group_img(nm_boxes[i]).copyTo(group_img);
            copyMakeBorder(group_img,group_img,15,15,15,15,BORDER_CONSTANT,Scalar(0));

            vector<Rect>   boxes;
            vector<string> words;
            vector<float>  confidences;
            ocr->run(group_img, output, &boxes, &words, &confidences, OCR_LEVEL_WORD);
            // trim() string
            output.erase(remove(output.begin(), output.end(), '\n'), output.end());
            //cout << "OCR output = \"" << output << "\" lenght = " << output.size() << endl;
            if (output.size() < 3)
                continue;

            // Sort boxes top-down and left-right
            std::sort(boxes.begin(), boxes.end(), compare_rect);

            for (int j=0; j<(int)boxes.size(); j++)
            {
                boxes[j].x += nm_boxes[i].x-15;
                boxes[j].y += nm_boxes[i].y-15;

                cout << "  word = " << words[j] << "\t confidence = " << confidences[j] << endl;
                if ((words[j].size() < 2) || (confidences[j] < 51) ||
                        ((words[j].size()==2) && (words[j][0] == words[j][1])) ||
                        ((words[j].size()< 4) && (confidences[j] < 60)) ||
                        isRepetitive(words[j]))
                    continue;
                words_detection.push_back(words[j]);
                // The position of the text within the page is likely to give information about its context
                    // The intersections will help in finding the block with the greatest text presence in one single loop for each text.
                Rect x_prev_intersection;
                Rect y_prev_intersection;
                Point2i block;
                for (int k=0; k<grid_dim;k++)
                {
                    Rect x_intersection=blocks[0][k] & boxes[j];
                    Rect y_intersection=blocks[1][k] & boxes[j];
                    if(x_intersection.area() > x_prev_intersection.area())
                    {
                        block.x=k;
                        x_prev_intersection=x_intersection;
                    }
                    if(y_intersection.area() > y_prev_intersection.area())
                    {
                        block.y=k;
                        y_prev_intersection=y_intersection;
                    }
                }
                words_detection_blocks.push_back(block);
                rectangle(out_img, boxes[j].tl(), boxes[j].br(), Scalar(255,0,255),3);
                Size word_size = getTextSize(words[j], FONT_HERSHEY_SIMPLEX, (double)scale_font, (int)(3*scale_font), NULL);
                rectangle(out_img, boxes[j].tl()-Point(3,word_size.height+3), boxes[j].tl()+Point(word_size.width,0), Scalar(255,0,100),-1);
                putText(out_img, words[j], boxes[j].tl()-Point(1,1), FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255,255,255),(int)(3*scale_font));
                out_img_segmentation = out_img_segmentation | group_segmentation;
            }

        }
        out_img.copyTo(out_img_detection);
    }
    else{
        /*Text Recognition (OCR)*/
        cout << "SIMPLE IMAGE DETECTED"<< endl;

        t_r = (double)getTickCount();

        Ptr<OCRTesseract> ocr = OCRTesseract::create(NULL, "spa");
        //Ptr<OCRTesseract> ocr = OCRTesseract::create();

        cout << "TIME_OCR_INITIALIZATION = " << ((double)getTickCount() - t_r)*1000/getTickFrequency() << endl;
        string output;

        Mat out_img;
        Mat out_img_segmentation = Mat::zeros(image.rows+2, image.cols+2, CV_8UC1);
        image.copyTo(out_img);
        image.copyTo(out_img_detection);
        float scale_img  = 600.f/image.rows;
        float scale_font = (float)(2-scale_img)/1.4f;

        t_r = (double)getTickCount();

        vector<Rect>   boxes;
        vector<string> words;
        vector<float>  confidences;
        ocr->run(out_img_detection, output, &boxes, &words, &confidences, OCR_LEVEL_WORD);
        // trim() string
        output.erase(remove(output.begin(), output.end(), '\n'), output.end());
        //cout << "OCR output = \"" << output << "\" lenght = " << output.size() << endl;

        // Sort boxes top-down and left-right
        std::sort(boxes.begin(), boxes.end(), compare_rect);

        for (int j=0; j<(int)boxes.size(); j++)
        {

            cout << "  word = " << words[j] << "\t confidence = " << confidences[j] << endl;
            if ((words[j].size() < 2) || (confidences[j] < 51) ||
                    ((words[j].size()==2) && (words[j][0] == words[j][1])) ||
                    ((words[j].size()< 4) && (confidences[j] < 60)) ||
                    isRepetitive(words[j]))
                continue;
            words_detection.push_back(words[j]);
            // The position of the text within the page is likely to give information about its context
                // The intersections will help in finding the block with the greatest text presence in one single loop for each text.
            Rect x_prev_intersection;
            Rect y_prev_intersection;
            Point2i block;
            for (int k=0; k<grid_dim;k++)
            {
                Rect x_intersection=blocks[0][k] & boxes[j];
                Rect y_intersection=blocks[1][k] & boxes[j];
                if(x_intersection.area() > x_prev_intersection.area())
                {
                    block.x=k;
                    x_prev_intersection=x_intersection;
                }
                if(y_intersection.area() > y_prev_intersection.area())
                {
                    block.y=k;
                    y_prev_intersection=y_intersection;
                }
            }
            words_detection_blocks.push_back(block);
            rectangle(out_img, boxes[j].tl(), boxes[j].br(), Scalar(255,0,255),3);
            Size word_size = getTextSize(words[j], FONT_HERSHEY_SIMPLEX, (double)scale_font, (int)(3*scale_font), NULL);
            rectangle(out_img, boxes[j].tl()-Point(3,word_size.height+3), boxes[j].tl()+Point(word_size.width,0), Scalar(255,0,100),-1);
            putText(out_img, words[j], boxes[j].tl()-Point(1,1), FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255,255,255),(int)(3*scale_font));
            out_img.copyTo(out_img_detection);
        }
    }

    for (vector<string>::iterator it = words_detection.begin() ; it != words_detection.end(); ++it) {
        //cout << "OCR Detection output = \"" << *it << "\" lenght = " << (*it).size() << endl;
    }

    cout << "TIME_OCR = " << ((double)getTickCount() - t_r)*1000/getTickFrequency() << endl;

    return out_img_detection;

}

Mat Omens::textDetection(const Mat &image) {
    bool lbp_flag = true;
    return textDetection(image,lbp_flag);
}


QImage Omens::textDetection(const QImage &img) {
    Mat image = image2Mat(img, CV_8UC3);
    return mat2Image(textDetection(image));
}

QPixmap Omens::textDetection(const QPixmap *pixMap) {
    Mat image = image2Mat(pixMap->toImage(), CV_8UC3);
    QImage res_img = mat2Image(textDetection(image));
    return QPixmap::fromImage(res_img);
}

QString Omens::getWordsDetection() {
    QString content;
    for (int i=0; i<(int)words_detection.size(); i++){
        content.append(QString::fromStdString(words_detection[i])+"\t"+QString::number(words_detection_blocks[i].x)+","+QString::number(words_detection_blocks[i].y)+"\n");
    }
//    foreach (string word, words_detection) {
//        content.append(QString::fromStdString(word)+"\n");
//    }
    return content;
}

bool compare_rect(const Rect & l, const Rect &r) {
    if(l.y == r.y) return l.x < r.x;
    return (l.y < r.y);
}

bool lbp_isComplex(const string &path_tempImage) {
    path absolutePath_script = absolute("../resources/python/lbp_isComplex.py");
    path absolutePath_model = absolute("../resources/python/model.cpickle");
    string command = "python " + absolutePath_script.string() + " " + path_tempImage + " " + absolutePath_model.string();
    int ret = system(command.c_str());
    if (ret) return true;
    else return false;
}

bool Omens::isRepetitive(const string& s)
{
    int count = 0;
    for (int i=0; i<(int)s.size(); i++)
    {
        if ((s[i] == 'i') ||
                (s[i] == 'l') ||
                (s[i] == 'I'))
            count++;
    }
    if (count > ((int)s.size()+1)/2)
    {
        return true;
    }
    return false;
}


void Omens::er_draw(vector<Mat> &channels, vector<vector<ERStat> > &regions, vector<Vec2i> group, Mat& segmentation)
{
    for (int r=0; r<(int)group.size(); r++)
    {
        ERStat er = regions[group[r][0]][group[r][1]];
        if (er.parent != NULL) // deprecate the root region
        {
            int newMaskVal = 255;
            int flags = 4 + (newMaskVal << 8) + FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
            floodFill(channels[group[r][0]],segmentation,Point(er.pixel%channels[group[r][0]].cols,er.pixel/channels[group[r][0]].cols),
                      Scalar(255),0,Scalar(er.level),Scalar(0),flags);
        }
    }
}

