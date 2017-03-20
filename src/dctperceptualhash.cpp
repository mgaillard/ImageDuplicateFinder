#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QDebug>

#include "dctperceptualhash.h"

quint64 DctPerceptualHash(const QString& file_path)
{
    quint64 hash = 0;

    cv::Mat input;
    cv::Mat grayImg;
    cv::Mat bluredImg;
    cv::Mat resizeImg;
    cv::Mat resizeFImg;
    cv::Mat dctImg;

    input = cv::imread(file_path.toStdString());

    if (input.data == NULL || (input.type() != CV_8UC3 && input.type() != CV_8U)) {
        // If there is a reading error, return a null hash.
        qDebug() << "Error reading file: " << file_path;
        return 0;
    }

    // Convert the image to grayscale using its luminance
    if(input.type() == CV_8UC3) {
        cv::cvtColor(input, grayImg, CV_BGR2GRAY);
    } else {
        grayImg = input;
    }

    // Mean filter with kernel 7x7.
    cv::blur(grayImg, bluredImg, cv::Size(7, 7));

    // Resize the image to 32x32 pixels.
    cv::resize(bluredImg, resizeImg, cv::Size(32,32));

    resizeImg.convertTo(resizeFImg, CV_32F);
    cv::dct(resizeFImg, dctImg);

    std::vector<float> topLeftDCTCoeffs;
    // Take only the coefficients in the rectangle (1, 1) ; (9, 9).
    topLeftDCTCoeffs.reserve(64);
    for (int i = 1; i <= 8; i++) {
        for (int j = 1; j <= 8; j++) {
            topLeftDCTCoeffs.push_back(dctImg.at<float>(i, j));
        }
    }

    // Compute the median
    std::vector<float> coeffs(topLeftDCTCoeffs);
    std::nth_element(coeffs.begin(), coeffs.begin() + coeffs.size()/2, coeffs.end());
    float median = (coeffs[31] + coeffs[32])/2;

    // Transform into a 64 bits integer.
    for (unsigned int i = 0; i < topLeftDCTCoeffs.size(); i++) {
        hash <<= 1;
        if (topLeftDCTCoeffs[i] >= median) {
            hash |= 1;
        }
    }

    return hash;
}

int DctPerceptualHashDistance(quint64 x, quint64 y)
{
    return __builtin_popcountll(x ^ y);
}
