#pragma once
#include <string>
#include "opencv2/slideio/slide.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS ImageDriver
        {
        public:
            virtual std::string getID() const = 0;
            virtual bool canOpenFile(const std::string& filePath) const = 0;
            virtual cv::Ptr<Slide> openFile(const std::string& filePath) = 0;
        };
    }
}

