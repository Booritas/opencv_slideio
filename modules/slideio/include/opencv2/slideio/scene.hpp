// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_scene_HPP
#define OPENCV_slideio_scene_HPP

#include "opencv2/slideio/structs.hpp"
#include "opencv2/core.hpp"
#include <vector>
#include <string>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W Scene
        {
        public:
            virtual ~Scene() = default;
            CV_WRAP virtual std::string getScenePath() const = 0;
            CV_WRAP virtual cv::Rect getSceneRect() const = 0;
            CV_WRAP virtual int getNumChannels() const = 0;
            CV_WRAP virtual cv::slideio::DataType getChannelDataType(int channel) const = 0;
            CV_WRAP virtual std::string getChannelName(int channel) const;
            CV_WRAP virtual Resolution  getResolution() const = 0;
            CV_WRAP virtual double getMagnification() const = 0;
            CV_WRAP virtual void readBlock(const cv::Rect& blockRect, cv::OutputArray output);
            CV_WRAP virtual void readBlockChannels(const cv::Rect& blockRect, const std::vector<int>& channelIndices, cv::OutputArray output);
            CV_WRAP virtual void readResampledBlock(const cv::Rect& blockRect, const cv::Size& blockSize, cv::OutputArray output);
            CV_WRAP virtual void readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize, const std::vector<int>& channelIndices, cv::OutputArray output) = 0;
            CV_WRAP virtual std::string getName() const = 0;
        };

    }
}
#endif