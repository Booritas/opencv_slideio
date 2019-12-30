#pragma once
#include "opencv2/slideio/slide.hpp"
#include <string>
#include <memory>
#include <list>

namespace cv
{
    namespace  slideio
    {
        std::shared_ptr<Slide> openSlide(const std::string& path, const std::string& driver);
        std::list<std::string> getDrivers();
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