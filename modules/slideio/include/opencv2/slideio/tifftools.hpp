// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_tifftools_HPP
#define OPENCV_slideio_tifftools_HPP

#include "opencv2/slideio/structs.hpp"
#include "opencv2/core.hpp"

#include <string>
#include <vector>

struct tiff;
typedef tiff TIFF;

namespace cv
{
    namespace slideio
    {
        struct TiffDirectory
        {
            int width;
            int height;
            bool tiled;
            int tileWidth;
            int tileHeight;
            int channels;
            int bitsPerSample;
            uint32_t compression;
            int dirIndex;
            int64 offset;
            std::string description;
            std::vector<TiffDirectory> subdirectories;
            Resolution res;
            cv::Point2d position;
            bool interleaved;
            int rowsPerStrip;
            DataType dataType;
            int stripSize;
        };
        class CV_EXPORTS  TiffTools
        {
        public:
            static TIFF* openTiffFile(const std::string& path);
            static void closeTiffFile(TIFF* file);
            static void scanTiffDirTags(TIFF* tiff, int dirIndex, int64_t dirOffset, slideio::TiffDirectory& dir);
            static void scanTiffDir(TIFF* tiff, int dirIndex, int64_t dirOffset, slideio::TiffDirectory& dir);
            static void scanFile(TIFF* file, std::vector<TiffDirectory>& directories);
            static void scanFile(const std::string& filePath, std::vector<TiffDirectory>& directories);
            static void readStripedDir(TIFF* file, const slideio::TiffDirectory& dir, cv::OutputArray output);
            static void readTile(TIFF* hFile, const slideio::TiffDirectory& dir, int tile,
                const std::vector<int>& channelIndices, cv::OutputArray output);
            static void setCurrentDirectory(TIFF* hFile, const slideio::TiffDirectory& dir);
            static void readJ2KTile(TIFF* hFile, const slideio::TiffDirectory& dir, int tile,
                const std::vector<int>& channelIndices, cv::OutputArray output);
            static void readRegularTile(TIFF* hFile, const slideio::TiffDirectory& dir, int tile,
                const std::vector<int>& channelIndices, cv::OutputArray output);
        };
    }
}
#endif