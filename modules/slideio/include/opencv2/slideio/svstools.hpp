// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_svstools_HPP
#define OPENCV_slideio_svstools_HPP

#include "opencv2/core.hpp"
#include <string>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS SVSTools
        {
        public:
            static int extractMagnifiation(const std::string& description);
        };
    }
}
#endif