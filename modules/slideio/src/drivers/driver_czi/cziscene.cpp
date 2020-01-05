// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/cziscene.hpp"
#include <boost/format.hpp>
#include <map>
#include "opencv2/slideio/czislide.hpp"
#include "opencv2/slideio/tilecomposer.hpp"
#include "opencv2/slideio/tools.hpp"

using namespace cv::slideio;
const double DOUBLE_EPSILON = 1.e-4;

// comparison function for zoom levels
class double_less : public std::binary_function<double, double, bool>
{
public:
    bool operator()(const double& left, const double& right) const
    {
        return (abs(left - right) > DOUBLE_EPSILON) && (left < right);
    }
};

CZIScene::CZIScene() : m_slide(nullptr)
{
}

std::string CZIScene::getScenePath() const
{
    return m_filePath;
}

cv::Rect CZIScene::getSceneRect() const
{
    return m_sceneRect;
}

int CZIScene::getNumChannels() const
{
    return static_cast<int>(m_channelInfos.size());
}

cv::slideio::DataType CZIScene::getChannelDataType(int channel) const
{
    if (channel < 0 || channel >= getNumChannels())
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Invalid channel index: %1%") % channel).str());
    }
    return m_channelInfos[channel].dataType;
}

std::string CZIScene::getChannelName(int channel) const
{
    if (channel < 0 || channel >= getNumChannels())
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Invalid channel index: %1%") % channel).str());
    }
    return m_channelInfos[channel].name;
}

Resolution CZIScene::getResolution() const
{
    return m_slide->getResolution();
}

double CZIScene::getMagnification() const
{
    return m_slide->getMagnification();
}

void CZIScene::readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
    const std::vector<int>& channelIndices, cv::OutputArray output)
{
    TilerData userData = { 0 };
    double zoom = static_cast<double>(blockSize.width) / static_cast<double>(blockRect.width);
    const auto& zoomLevels = m_zoomLevels;
    userData.zoomLevelIndex = Tools::findZoomLevel(zoom, static_cast<int>(m_zoomLevels.size()), [&zoomLevels](int index){
        return zoomLevels[index].zoom;
    });
    TileComposer::composeRect(this, channelIndices, blockRect, blockSize, output, &userData);
}

std::string CZIScene::getName() const
{
    return m_name;
}

void CZIScene::generateSceneName()
{
    int s(0), i(0), v(0), h(0), r(0), b(0);
    CZISlide::dimsFromSceneId(m_id, s, i, v, h, r, b);
    m_name = (boost::format("%1%(s:%2% i:%3% v:%4% h:%5% r:%6% b:%7%)")
        % m_slide->getTitle()
        % s % i % v % h % r % b).str();
}

void CZIScene::init(uint64_t sceneId, const std::string& filePath, const Blocks& blocks, CZISlide* slide)
{
    m_slide = slide;
    m_id = sceneId;
    // separate blocks by zoom levels and detect count of channels and channel data type
    m_filePath = filePath;
    std::map<double, int, double_less> zoomLevelIndices;
    std::map<int, int> channelPixelType;
    for(const auto& block : blocks)
    {
        double zoom = block.zoom;
        int zoomLevelIndex = 0;
        auto itIndex = zoomLevelIndices.find(zoom);
        if(itIndex==zoomLevelIndices.end())
        {
            zoomLevelIndex = static_cast<int>(zoomLevelIndices.size());
            zoomLevelIndices[zoom] = zoomLevelIndex;
            m_zoomLevels.emplace_back();
            m_zoomLevels.back().zoom = zoom;
        }
        for(int channelIndex=block.firstChannel; channelIndex<=block.lastChannel; channelIndex++)
        {
            channelPixelType[channelIndex] = block.pixelType;
        }
        m_zoomLevels[zoomLevelIndex].blocks.push_back(block);
    }
    setupChannels(channelPixelType);
    // sort zoom levels in ascending order
    std::sort(m_zoomLevels.begin(), m_zoomLevels.end(), [](const ZoomLevel& left, const ZoomLevel& right)
    {
        return (abs(left.zoom - right.zoom) > DOUBLE_EPSILON) && (left.zoom > right.zoom);
    });
    // combine zoom level blocks in tiles
    for(auto& zoomLevel: m_zoomLevels)
    {
        combineBlockInTiles(zoomLevel);
    }
    // compute scene rectangle
    const ZoomLevel& zoomLevelMax = m_zoomLevels.front();
    if(abs(zoomLevelMax.zoom-1)>1.e-4)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: unexpected value for max zoom level. Expected: 1, received: %1%") %
                zoomLevelMax.zoom).str());
    }
    m_sceneRect = { 0,0,0,0 };
    const Tiles& tiles = zoomLevelMax.tiles;
    const Blocks& tileBlocks = zoomLevelMax.blocks;
    for(size_t index = 0; index<tiles.size(); index++)
    {
        int blockIndex = tiles[index].blockIndices[0];
        const Block& block = tileBlocks[blockIndex];
        cv::Rect tileRect = { block.x, block.y, block.width, block.height };
        m_sceneRect |= tileRect;
    }
    generateSceneName();
}

int CZIScene::getTileCount(void* userData)
{
    return 0;
}

bool CZIScene::getTileRect(int tileIndex, cv::Rect& tileRect, void* userData)
{
    return false;
}

bool CZIScene::readTile(int tileIndex, const std::vector<int>& channelIndices, cv::OutputArray tileRaster,
    void* userData)
{
    return false;
}


void CZIScene::combineBlockInTiles(ZoomLevel& zoomLevel)
{
    std::map<uint64_t, int> coordsToIndex;
    Tiles& tiles = zoomLevel.tiles;
    for(int blockIndex=0; blockIndex < static_cast<int>(zoomLevel.blocks.size()); blockIndex++)
    {
        const Block& block = zoomLevel.blocks[blockIndex];
        uint64_t key = (static_cast<uint64_t>(block.x) << 32) | block.y;
        auto tileIt = coordsToIndex.find(key);
        int index;
        if (tileIt == coordsToIndex.end())
        {
            index = (int)tiles.size();
            coordsToIndex[key] = index;
            tiles.emplace_back();
        }
        else
        {
            index = tileIt->second;
        }
        tiles[index].blockIndices.push_back(blockIndex);
    }
}

void CZIScene::setupChannels(const std::map<int, int>& channelPixelType)
{
    int imageGlobalChannel = 0;
    const CZIChannelInfos& fileChannelInfo = m_slide->getChannelInfo();
    for(const auto& channel : channelPixelType)
    {
        int channelIndex = channel.first;
        CZIDataType channelCZIDataType = static_cast<CZIDataType>(channel.second);
        int imageChannels;
        DataType dt = DataType::DT_Unknown;
        switch(channelCZIDataType)
        {
            case Gray8:
                dt = DataType::DT_Byte;
                imageChannels = 1;
            break;
            case Gray16:
                dt = DataType::DT_UInt16;
                imageChannels = 1;
                break;
            case Gray32Float:
                dt = DataType::DT_Float32;
                imageChannels = 1;
                break;
            case Bgr24:
                dt = DataType::DT_Byte;
                imageChannels = 3;
                break;
            case Bgr48:
                dt = DataType::DT_UInt16;
                imageChannels = 3;
                break;
            case Bgr96Float:
                dt = DataType::DT_Float32;
                imageChannels = 3;
                break;
            case Bgra32:
                dt = DataType::DT_Byte;
                imageChannels = 4;
                break;
            case Gray32:
                dt = DataType::DT_Int32;
                imageChannels = 1;
                break;
            case Gray64:
                dt = DataType::DT_Float64;
                imageChannels = 1;
                break;
            case Bgr192ComplexFloat:
            case Gray64ComplexFloat:
            default:
                throw std::runtime_error(
                    (boost::format("CZIImageDriver: Unsupported data type: %1%") % channelCZIDataType).str());
        }
        for(int imageChannel=0; imageChannel<imageChannels; ++imageChannel, ++imageGlobalChannel)
        {
            m_imageChannelToFileChannel[imageGlobalChannel] = std::pair<int,int>(channelIndex, imageChannel);
            m_channelInfos.emplace_back();
            auto& channelInfo = m_channelInfos.back();
            channelInfo.dataType = dt;
            if(channelIndex<fileChannelInfo.size())
            {
                if(imageChannels==1)
                {
                    channelInfo.name = fileChannelInfo[channelIndex].id;
                }
                else
                {
                    channelInfo.name = (boost::format("%1%:%2%") % fileChannelInfo[channelIndex].id % (imageChannel+1)).str();
                }
            }
        }
    }
}
