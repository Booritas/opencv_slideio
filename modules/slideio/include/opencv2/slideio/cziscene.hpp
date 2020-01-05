// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_cziscene_HPP
#define OPENCV_slideio_cziscene_HPP
#include "scene.hpp"
#include "czistructs.hpp"
#include "opencv2/slideio/tilecomposer.hpp"
#include <map>

namespace cv
{
    namespace slideio
    {
        class CZISlide;
        class CV_EXPORTS CZIScene : public Scene, public Tiler
        {
            enum CZIDataType
            {
                Gray8 = 0,
                Gray16 = 1,
                Gray32Float = 2,
                Bgr24 = 3,
                Bgr48 = 4,
                Bgr96Float = 8,
                Bgra32 = 9,
                Gray64ComplexFloat = 10,
                Bgr192ComplexFloat = 11,
                Gray32 = 12,
                Gray64 = 13,
            };
            struct Tile
            {
                std::vector<int> blockIndices;
            };
            typedef std::vector<Tile> Tiles;
            struct ZoomLevel
            {
                double zoom;
                Blocks blocks;
                Tiles tiles;
            };
            struct ChannelInfo
            {
                std::string name;
                DataType dataType;
            };
            struct TilerData
            {
                int zoomLevelIndex;
                int zSliceIndex;
                int tFrameIndex;
            };
        public:
            CZIScene();
            std::string getScenePath() const override;
            cv::Rect getSceneRect() const override;
            int getNumChannels() const override;
            cv::slideio::DataType getChannelDataType(int channel) const override;
            std::string getChannelName(int channel) const override;
            Resolution getResolution() const override;
            double getMagnification() const override;
            void readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
                const std::vector<int>& channelIndices, cv::OutputArray output) override;
            std::string getName() const override;
            void generateSceneName();
            void init(uint64_t sceneId, const std::string& filePath, const Blocks& blocks, CZISlide* slide);
            // interface Tiler implementaton
            int getTileCount(void* userData) override;
            bool getTileRect(int tileIndex, cv::Rect& tileRect, void* userData) override;
            bool readTile(int tileIndex, const std::vector<int>& channelIndices, cv::OutputArray tileRaster,
                void* userData) override;
        private:
            static void combineBlockInTiles(ZoomLevel& zoomLevel);
            void setupChannels(const std::map<int, int>& channelPixelType);
        private:
            std::vector<ZoomLevel> m_zoomLevels;
            std::vector<ChannelInfo> m_channelInfos;
            std::string m_filePath;
            cv::Rect m_sceneRect;
            std::map<int, std::pair<int, int>> m_imageChannelToFileChannel;
            CZISlide* m_slide;
            std::string m_name;
            uint64_t m_id;
        };
    }
}

#endif
