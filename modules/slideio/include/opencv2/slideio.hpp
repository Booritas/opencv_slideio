#pragma once
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