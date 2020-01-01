// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_cziscene_HPP
#define OPENCV_slideio_cziscene_HPP
#include "scene.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS CZIScene : public Scene
        {
        public:
            std::string getScenePath() const override;
            cv::Rect getSceneRect() const override;
            int getNumChannels() const override;
            cv::slideio::DataType getChannelDataType(int channel) const override;
            Resolution getResolution() const override;
            double getMagnification() const override;
            void readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
                const std::vector<int>& channelIndices, cv::OutputArray output) override;
            std::string getName() const override;
        };
    }
}

#endif
