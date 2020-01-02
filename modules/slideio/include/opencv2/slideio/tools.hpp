// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_tools_HPP
#define OPENCV_slideio_tools_HPP

#include "opencv2/slideio/structs.hpp"
#include "opencv2/core.hpp"
#include <vector>
#include <string>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS Tools
        {
        public:
            static bool matchPattern(const std::string& path, const std::string& pattern);
        };

    }
}
#endif