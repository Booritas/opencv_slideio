#include "opencv2/slideio/scene.hpp"

using namespace cv::slideio;

void Scene::readBlock(const cv::Rect& blockRect, cv::OutputArray output)
{
    const std::vector<int> channelIndices;
    return readBlockChannels(blockRect, channelIndices, output);
}

void Scene::readBlockChannels(const cv::Rect& blockRect, const std::vector<int>& channelIndices, cv::OutputArray output)
{
    const cv::Rect rectScene = getSceneRect();
    return readResampledBlockChannels(blockRect, blockRect.size(), channelIndices, output);
}

void Scene::readResampledBlock(const cv::Rect& blockRect, const cv::Size& blockSize, cv::OutputArray output)
{
    const std::vector<int> channelIndices;
    return readResampledBlockChannels(blockRect, blockSize, channelIndices, output);
}
