#include "../include/PaddedMat.h"
using namespace cv;

PaddedMat::PaddedMat(const Mat &mat) : Mat(pad(mat)) {}

PaddedMat::PaddedMat(const PaddedMat &other) : Mat(other)
{
    rowDiff = other.rowDiff;
    colDiff = other.colDiff;
}

PaddedMat::PaddedMat &operator=(const PaddedMat &other)
{
    if (this != &other)
    {
        Mat::operator=(other);
        rowDiff = other.rowDiff;
        colDiff = other.colDiff;
    }
    return *this;
}

Mat PaddedMat::pad(const Mat &mat)
{
    unsigned int larger = std::max(mat.rows, mat.cols);
    unsigned int desire = std::pow(2, std::ceil(std::log2(larger)));
    rowDiff = desire - mat.rows;
    colDiff = desire - mat.cols;

    Mat paddedMat = Mat::zeros(desire, desire, mat.type());
    mat.copyTo(paddedMat(Rect(0, 0, mat.cols, mat.rows)));

    return paddedMat;
}
