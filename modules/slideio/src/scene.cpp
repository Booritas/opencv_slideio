#include "opencv2/slideio/scene.hpp"

using namespace cv::slideio;

void Scene::readBlock(const cv::Rect& blockRect, cv::OutputArray output)
{
    const std::vector<int> channelIndices;
    return readBlock(blockRect, channelIndices, output);
}

void Scene::readBlock(const cv::Rect& blockRect, const std::vector<int>& channelIndices, cv::OutputArray output)
{
    const cv::Rect rectScene = getSceneRect();
    return readBlock(blockRect, blockRect.size(), channelIndices, output);
}

void Scene::readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, cv::OutputArray output)
{
    const std::vector<int> channelIndices;
    return readBlock(blockRect, blockSize, channelIndices, output);
}
