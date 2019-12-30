#pragma once
#include "opencv2/slideio/scene.hpp"
#include "opencv2/core.hpp"
#pragma warning( push )
#pragma warning(disable:4005)
#include <gdal/gdal.h>
#pragma warning( pop )

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W GDALScene : public slideio::Scene
        {
        public:
            GDALScene(const std::string& filePath);
            GDALScene(GDALDatasetH ds, const std::string& filePath);
            virtual ~GDALScene();
            std::string getScenePath() const override;
            int getNumChannels() const override;
            slideio::DataType getChannelDataType(int channel) const override;
            slideio::Resolution getResolution() const override;
            double getMagnification() const override;
            static GDALDatasetH openFile(const std::string& filePath);
            static void closeFile(GDALDatasetH hfile);
            static slideio::DataType dataTypeFromGDALDataType(GDALDataType dt);
            std::string getName() const override;
            cv::Rect getSceneRect() const override;
            void readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, const std::vector<int>& channelIndices, cv::OutputArray output) override;
        private:
            GDALDatasetH m_hFile;
            std::string m_filePath;
        };
    }
}


