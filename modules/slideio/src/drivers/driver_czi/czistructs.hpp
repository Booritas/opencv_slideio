// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_czistructs_HPP
#define OPENCV_slideio_czistructs_HPP
#include <cstdint>

namespace cv
{
    namespace slideio
    {
        #pragma pack(push,1)
        struct SegmentHeader
        {
            char SID[16];
            uint64_t allocatedSize;
            uint64_t usedSize;
        };
        struct FileHeader
        {
            uint32_t majorVersion;
            uint32_t minorVerion;
            uint64_t reserved;
            char primaryFileGuid[16];
            char fileGuid[16];
            uint32_t filePart;
            uint64_t directoryPosition;
            uint64_t metadataPosition;
            uint32_t updatePending;
            uint64_t attachmentDirectoryPosition;
        };
        struct MetadataHeader
        {
            uint32_t xmlSize;
            uint32_t attachmentSize;
            uint8_t reserved[248];
        };
        struct DirectoryHeader
        {
            uint32_t entryCount;
            uint8_t reserved[124];
        };
        struct DirectoryEntryDV
        {
            char schemaType[2];
            int32_t pixelType;
            int64_t filePosition;
            int32_t filePart;
            int32_t compression;
            uint8_t pyramidType;
            uint8_t reserved[5];
            int32_t dimensionCount;
        };
        struct DimensionEntryDV
        {
            char dimension[4];
            int32_t start;
            int32_t size;
            float startCoordinate;
            int32_t storedSize;
        };
        #pragma pack(pop)
    }
}
#endif
