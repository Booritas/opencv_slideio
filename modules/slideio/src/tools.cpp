// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/tools.hpp"
#if defined(WIN32)
#include <Shlwapi.h>
#else
#include <fnmatch.h>
#endif
using namespace cv::slideio;

bool Tools::matchPattern(const std::string& path, const std::string& pattern)
{
    //TODO:: Implement Linux version for fnmatch
    bool ret(false);
#if defined(WIN32)
    ret = PathMatchSpecA(path.c_str(), pattern.c_str())!=0;
#else
    put fnmatch call here
    throw std::runtime_error("Fnmatch is not yet implemented")
#endif

    return ret;
}
