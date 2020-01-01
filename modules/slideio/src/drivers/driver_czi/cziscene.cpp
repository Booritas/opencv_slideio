// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/cziscene.hpp"

using namespace cv::slideio;


std::string CZIScene::getScenePath() const
{
    return "";
}

cv::Rect CZIScene::getSceneRect() const
{
    return cv::Rect();
}

int CZIScene::getNumChannels() const
{
    return 0;
}

cv::slideio::DataType CZIScene::getChannelDataType(int channel) const
{
    return DataType::DT_Unknown;
}

Resolution CZIScene::getResolution() const
{
    return Resolution();
}

double CZIScene::getMagnification() const
{
    return 0;
}

void CZIScene::readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
    const std::vector<int>& channelIndices, cv::OutputArray output)
{
}

std::string CZIScene::getName() const
{
    return "";
}
