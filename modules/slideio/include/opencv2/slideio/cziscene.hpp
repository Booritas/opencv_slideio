// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_cziscene_HPP
#define OPENCV_slideio_cziscene_HPP
#include "scene.hpp"
#include <fstream>

namespace tinyxml2
{
    class XMLNode;
    class XMLElement;
    class XMLDocument;
}
namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS CZIScene : public Scene
        {
            struct Channel
            {
                std::string id;
                DataType dataType;
                int componentCount;
            };
            struct Dimension
            {
                char type;
                int start;
                int size;
            };
            struct Block
            {
                int32_t pixelType;
                int64_t filePosition;
                int32_t filePart;
                int32_t compression;
                int32_t x;
                int32_t y;
                int32_t width;
                int32_t height;
                int32_t scene;
                double zoom;
                std::vector<Dimension> dimensions;
            };
        public:
            CZIScene();
            std::string getScenePath() const override;
            cv::Rect getSceneRect() const override;
            int getNumChannels() const override;
            cv::slideio::DataType getChannelDataType(int channel) const override;
            Resolution getResolution() const override;
            double getMagnification() const override;
            void readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
                const std::vector<int>& channelIndices, cv::OutputArray output) override;
            std::string getName() const override;
            void init(const std::string& filePath);
        private:
            void readMetadata();
            void readFileHeader();
            void readDirectory();
            void parseMagnification(tinyxml2::XMLNode* root);
            void parseMetadataXmL(const char* xml, size_t dataSize);
            void parseResolutions(tinyxml2::XMLNode* root);
            void parseSizes(tinyxml2::XMLNode* root);
            void parseChannels(tinyxml2::XMLNode* root);
        private:
            std::string m_filePath;
            std::ifstream m_fileStream;
            uint64_t m_directoryPosition;
            uint64_t m_metadataPosition;
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
            double m_magnification;
            double m_resX;
            double m_resY;
            double m_resZ;
            std::vector<Channel> m_channels;
        };
    }
}

#endif
