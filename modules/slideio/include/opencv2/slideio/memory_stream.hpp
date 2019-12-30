#pragma once
#include "openjpeg.h"

namespace cv
{
    namespace slideio
    {
        class OPJStreamUserData {
        public:
            OPJStreamUserData(unsigned char* ptr, size_t sz) : data(ptr), size(sz), offset(0) {}
            unsigned char* data;
            size_t size;
            size_t offset;
        };
        opj_stream_t* createOPJMemoryStream(OPJStreamUserData* data, size_t size, bool inputStream);
    }
}