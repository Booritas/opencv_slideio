// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_slideio_HPP
#define OPENCV_slideio_slideio_HPP

#include "opencv2/slideio/slide.hpp"
#include <string>
#include <vector>

namespace cv
{
    namespace  slideio
    {
        CV_EXPORTS_W cv::Ptr<Slide> openSlide(const cv::String& path, const cv::String& driver);
        CV_EXPORTS_W std::vector<cv::String> getDrivers();
        inline DataType fromOpencvType(int type)
        {
            return static_cast<DataType>(type);
        }
        inline int toOpencvType(DataType dt)
        {
            return static_cast<int>(dt);
        }
        inline bool isValidDataType(slideio::DataType dt)
        {
            return dt <= slideio::DataType::DT_LastValid;
        }
    }
}
#endif