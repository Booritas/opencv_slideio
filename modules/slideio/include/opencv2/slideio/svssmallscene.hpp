#pragma once
#include "opencv2/slideio/svsscene.hpp"
#include "opencv2/slideio/tifftools.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS SVSSmallScene : public SVSScene
        {
        public:
            SVSSmallScene(
                const std::string& filePath,
                const std::string& name,
                const slideio::TiffDirectory& dir,
                TIFF* hfile);
            cv::Rect getSceneRect() const override;
            int getNumChannels() const override;
            slideio::DataType getChannelDataType(int channel) const override;
            slideio::Resolution getResolution() const override;
            double getMagnification() const override;
            void readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, const std::vector<int>& channelIndices,
                cv::OutputArray output) override;
        private:
            slideio::TiffDirectory m_directory;
            slideio::DataType m_dataType;
            double m_magnification;
            TIFF* m_hFile;
        };
    }
}