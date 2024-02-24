#ifndef PADDED_MAT_HPP
#define PADDED_MAT_HPP

#include <opencv2/opencv.hpp>

class PaddedMat : public cv::Mat {
public:
    unsigned int rowDiff, colDiff;

    PaddedMat(const cv::Mat& mat);
    PaddedMat(const PaddedMat& other);
    PaddedMat& operator=(const PaddedMat& other);

private:
    cv::Mat pad(const cv::Mat& mat)
};

#endif // PADDED_MAT_HPP
