// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_czislide_HPP
#define OPENCV_slideio_czislide_HPP
#include "slide.hpp"
#include <fstream>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS CZISlide : public Slide
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
            #pragma pack(pop)
        public:
            CZISlide(const std::string& filePath);
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            cv::Ptr<Scene> getScene(int index) const override;
        private:
            void init();
            void parseMetadataXmL(const char* xml, size_t dataSize);
            void readMetadata();
            void readFileHeader();
        private:
            std::string m_filePath;
            std::ifstream m_fileStream;
            FileHeader m_fileHeader;
            // image parameters
            int m_slideXs;
            int m_slideYs;
            int m_slideZs;
            int m_slideTs;
            int m_slideRs;
            int m_slideIs;
            int m_slideSs;
            int m_slideHs;
            int m_slideMs;
            int m_slideBs;
            int m_slideVs;
        };
    }
}

#endif
