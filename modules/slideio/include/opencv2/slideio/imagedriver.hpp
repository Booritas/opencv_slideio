// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_imagedriver_HPP
#define OPENCV_slideio_imagedriver_HPP

#include "opencv2/slideio/slide.hpp"
#include <string>

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
#endif
