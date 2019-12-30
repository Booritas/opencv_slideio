#pragma once
#include <opencv2/core.hpp>

namespace cv
{
    namespace slideio
    {
        enum class DataType
        {
            DT_Byte = CV_8U,
            DT_Int8 = CV_8S,
            DT_Int16 = CV_16S,
            DT_Float16 = CV_16F,
            DT_Int32 = CV_32S,
            DT_Float32 = CV_32F,
            DT_Float64 = CV_64F,
            DT_UInt16 = CV_16U,
            DT_LastValid = CV_16U,
            DT_Unknown = 1024,
            DT_None = 2048
        };
        typedef Point2d Resolution;
    }
}
