// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_cziscene_HPP
#define OPENCV_slideio_cziscene_HPP
#include "scene.hpp"
#include "czistructs.hpp"
#include "opencv2/slideio/tilecomposer.hpp"
#include <map>
#include "czisubblock.hpp"

namespace cv
{
    namespace slideio
    {
        class CZISlide;
        class CV_EXPORTS CZIScene : public Scene, public Tiler
        {
        public:
            struct SceneParams
            {
                int illuminationIndex;
                int bAccusitionIndex;
                int rotationIndex;
                int sceneIndex;
                int hPhaseIndex;
                int viewIndex;
            };
        private:
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
            struct ComponentInfo
            {
                std::string name;
                DataType dataType;
            };
            struct SceneChannelInfo
            {
                std::string name;
                int32_t pixelType;
                int32_t pixelSize;
                int32_t numComponents;
                DataType componentType;
            };
            struct TilerData
            {
                int zoomLevelIndex;
                int zSliceIndex;
                int tFrameIndex;
                double relativeZoom;
            };
        public:
            CZIScene();
            std::string getFilePath() const override;
            cv::Rect getRect() const override;
            int getNumChannels() const override;
            int getNumZSlices() const override;
            int getNumTFrames() const override;
            double getZSliceResolution() const override;
            double getTFrameResolution() const override;
            cv::slideio::DataType getChannelDataType(int channel) const override;
            std::string getChannelName(int channel) const override;
            Resolution getResolution() const override;
            double getMagnification() const override;
            void readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
                const std::vector<int>& componentIndices, cv::OutputArray output) override;
            std::string getName() const override;
            void init(uint64_t sceneId, SceneParams& sceneParams, const std::string& filePath, const Blocks& blocks, CZISlide* slide);
            // interface Tiler implementaton
            int getTileCount(void* userData) override;
            bool getTileRect(int tileIndex, cv::Rect& tileRect, void* userData) override;
            void readBlockChannel(const CZISubBlock& block, int cziChannel, int z, int t, cv::OutputArray output);
            bool readTile(int tileIndex, const std::vector<int>& componentIndices, cv::OutputArray tileRaster,
                          void* userData) override;
        private:
            void setupComponents(const std::map<int, int>& channelPixelType);
            void generateSceneName();
            void computeSceneRect();
            void computeSceneTiles();
            void compute4DParameters();
            const ZoomLevel& getBaseZoomLevel() const;
            int findBlockIndex(const Tile& tile, const Blocks& blocks, int channelIndex, int zSliceIndex, int tFrameIndex) const ;
        public:
            // static members
            static uint64_t sceneIdFromDims(int s, int i, int v, int h, int r, int b);
            static uint64_t sceneIdFromDims(const std::vector<Dimension>& dims);
            static void sceneIdsFromDims(const std::vector<Dimension>& dims, std::vector<uint64_t>& ids);
            static uint64_t sceneIdFromDims(const SceneParams& params);
            static void dimsFromSceneId(uint64_t sceneId, int& s, int& i, int& v, int& h, int& r, int& b);
            static void dimsFromSceneId(uint64_t sceneId, SceneParams& params);
            static void channelComponentInfo(CZIDataType channelType, DataType& componentType, int& numComponents, int& pixelSize);
        private:
            static void combineBlockInTiles(ZoomLevel& zoomLevel);
            // data members
        private:
            std::vector<ZoomLevel> m_zoomLevels;
            std::vector<ComponentInfo> m_componentInfos;
            std::vector<SceneChannelInfo> m_channelInfos;
            std::string m_filePath;
            cv::Rect m_sceneRect;
            std::map<int, std::pair<int, int>> m_componentToChannelIndex;
            CZISlide* m_slide;
            std::string m_name;
            uint64_t m_id{};
            SceneParams m_sceneParams{};
            int m_numZSlices;
            int m_numTFrames;
        };
    }
}

#endif
