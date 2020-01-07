// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_czislide_HPP
#define OPENCV_slideio_czislide_HPP
#include "slide.hpp"
#include <fstream>
#include "cziscene.hpp"
#include "czistructs.hpp"

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
        class CV_EXPORTS CZISlide : public Slide
        {
        public:
            CZISlide(const std::string& filePath);
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            cv::Ptr<Scene> getScene(int index) const override;
            double getMagnification() const { return m_magnification; }
            Resolution getResolution() const { return { m_resX, m_resY }; }
            const CZIChannelInfos& getChannelInfo() const { return m_channels; }
            const std::string& getTitle() const { return m_title; };
        private:
            void init();
            void readMetadata();
            void readFileHeader();
            void readDirectory();
            void parseMagnification(tinyxml2::XMLNode* root);
            void parseMetadataXmL(const char* xml, size_t dataSize);
            void parseResolutions(tinyxml2::XMLNode* root);
            void parseSizes(tinyxml2::XMLNode* root);
            void parseChannels(tinyxml2::XMLNode* root);
        private:
            std::vector<cv::Ptr<CZIScene>> m_scenes;
            std::string m_filePath;
            std::ifstream m_fileStream;
            uint64_t m_directoryPosition{};
            uint64_t m_metadataPosition{};
            // image parameters
            int m_slideXs{};
            int m_slideYs{};
            int m_slideZs{};
            int m_slideTs{};
            int m_slideRs{};
            int m_slideIs{};
            int m_slideSs{};
            int m_slideHs{};
            int m_slideMs{};
            int m_slideBs{};
            int m_slideVs{};
            double m_magnification{};
            double m_resX{};
            double m_resY{};
            double m_resZ{};
            CZIChannelInfos m_channels;
            std::string m_title;
        };
    }
}

#endif
