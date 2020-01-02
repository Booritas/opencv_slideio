// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_gdalimagedriver_HPP
#define OPENCV_slideio_gdalimagedriver_HPP

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
            cv::Ptr<Slide> openFile(const std::string& filePath) override;
            std::string getFileSpecs() const override;
        };
    }
}
#endif
