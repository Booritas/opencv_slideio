#pragma once
#include "opencv2/slideio/imagedriver.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS GDALImageDriver : public slideio::ImageDriver
        {
        public:
            GDALImageDriver();
            ~GDALImageDriver();
            std::string getID() const override;
            bool canOpenFile(const std::string& filePath) const override;
            cv::Ptr<Slide> openFile(const std::string& filePath) override;
        };
    }
}
