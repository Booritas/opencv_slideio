#pragma once
#include <string>
#include "opencv2/slideio/slide.hpp"

namespace cv
{
    namespace slideio
    {
        class ImageDriver
        {
        public:
            virtual std::string getID() const = 0;
            virtual bool canOpenFile(const std::string& filePath) const = 0;
            virtual std::shared_ptr<Slide> openFile(const std::string& filePath) = 0;
        };
    }
}

