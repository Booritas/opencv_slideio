#pragma once
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
            virtual std::string getScenePath() const = 0;
            virtual cv::Rect getSceneRect() const = 0;
            virtual int getNumChannels() const = 0;
            virtual DataType getChannelDataType(int channel) const = 0;
            virtual Resolution  getResolution() const = 0;
            virtual double getMagnification() const = 0;
            virtual void readBlock(const cv::Rect& blockRect, cv::OutputArray output);
            virtual void readBlock(const cv::Rect& blockRect, const std::vector<int>& channelIndices, cv::OutputArray output);
            virtual void readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, cv::OutputArray output);
            virtual void readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, const std::vector<int>& channelIndices, cv::OutputArray output) = 0;
            virtual std::string getName() const = 0;
        };

    }
}
