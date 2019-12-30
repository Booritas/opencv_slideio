#pragma once
#include "opencv2/slideio/imagedriver.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W SVSImageDriver : public cv::slideio::ImageDriver
        {
        public:
            SVSImageDriver();
            ~SVSImageDriver();
            std::string getID() const override;
            bool canOpenFile(const std::string& filePath) const override;
            std::shared_ptr<slideio::Slide> openFile(const std::string& filePath) override;
        };
    }
}
