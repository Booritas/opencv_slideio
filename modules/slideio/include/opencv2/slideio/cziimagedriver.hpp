// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_cziimagedriver_HPP
#define OPENCV_slideio_cziimagedriver_HPP
#include "imagedriver.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS CZIImageDriver : public slideio::ImageDriver
        {
        public:
            CZIImageDriver();
            ~CZIImageDriver();
            std::string getID() const override;
            bool canOpenFile(const std::string& filePath) const override;
            cv::Ptr<Slide> openFile(const std::string& filePath) override;
        };
    }
}

#endif
