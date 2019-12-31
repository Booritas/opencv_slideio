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
            CV_WRAP virtual std::string getScenePath() const = 0;
            CV_WRAP virtual cv::Rect getSceneRect() const = 0;
            CV_WRAP virtual int getNumChannels() const = 0;
            CV_WRAP virtual cv::slideio::DataType getChannelDataType(int channel) const = 0;
            CV_WRAP virtual Resolution  getResolution() const = 0;
            CV_WRAP virtual double getMagnification() const = 0;
            CV_WRAP virtual void readBlock(const cv::Rect& blockRect, cv::OutputArray output);
            CV_WRAP virtual void readBlock(const cv::Rect& blockRect, const std::vector<int>& channelIndices, cv::OutputArray output);
            CV_WRAP virtual void readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, cv::OutputArray output);
            CV_WRAP virtual void readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, const std::vector<int>& channelIndices, cv::OutputArray output) = 0;
            CV_WRAP virtual std::string getName() const = 0;
        };

    }
}
