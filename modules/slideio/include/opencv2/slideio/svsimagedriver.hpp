#pragma once
#include "opencv2/slideio/imagedriver.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS SVSImageDriver : public cv::slideio::ImageDriver
        {
        public:
            SVSImageDriver();
            ~SVSImageDriver();
            std::string getID() const override;
            bool canOpenFile(const std::string& filePath) const override;
            cv::Ptr<slideio::Slide> openFile(const std::string& filePath) override;
        };
    }
}
