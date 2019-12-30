#pragma once
#include "opencv2/core.hpp"
#include <string>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W SVSTools
        {
        public:
            static int extractMagnifiation(const std::string& description);
        };
    }
}