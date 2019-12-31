#pragma once
#include <opencv2/core.hpp>

namespace cv
{
    namespace slideio
    {
        class Tiler
        {
        public:
            virtual int getTileCount(void* userData) = 0;
            virtual bool getTileRect(int tileIndex, cv::Rect& tileRect, void* userData) = 0;
            virtual bool readTile(int tileIndex, const std::vector<int>& channelIndices, cv::OutputArray tileRaster, void* userData) = 0;
        };
        class CV_EXPORTS TileComposer
        {
        public:
            static void composeRect(Tiler* tiler, const std::vector<int>& channelIndices,
                const cv::Rect& blockRect, const cv::Size& blockSize, cv::OutputArray output, void* userData = nullptr);
        };
    }
}