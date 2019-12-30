#pragma once
#include "opencv2/slideio/structs.hpp"
#include "opencv2/core.hpp"

#include <string>
#include <vector>
#include <tiffio.h>

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
            TIFFDataType dataType;
            int stripSize;
        };
        class CV_EXPORTS_W TiffTools
        {
        public:
            static void scanTiffDirTags(TIFF* tiff, slideio::TiffDirectory& dir);
            static void scanTiffDir(TIFF* tiff, slideio::TiffDirectory& dir);
            static void scanFile(TIFF* file, std::vector<TiffDirectory>& directories);
            static void scanFile(const std::string& filePath, std::vector<TiffDirectory>& directories);
            static int dataTypeSize(TIFFDataType dt);
            static slideio::DataType dataTypeFromTIFFDataType(TIFFDataType dt);
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
