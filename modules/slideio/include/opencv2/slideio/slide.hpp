// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_slide_HPP
#define OPENCV_slideio_slide_HPP

#include <string>
#include "opencv2/slideio/scene.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W Slide
        {
        public:
            CV_WRAP virtual int getNumbScenes() const = 0;
            CV_WRAP virtual std::string getFilePath() const = 0;
            CV_WRAP virtual cv::Ptr<Scene> getScene(int index) const = 0;
        };
    }
}
#endif