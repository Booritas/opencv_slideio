﻿// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/cziscene.hpp"
#include <boost/format.hpp>
#include <map>
#include "opencv2/slideio/czislide.hpp"
#include "opencv2/slideio/tilecomposer.hpp"
#include "opencv2/slideio/tools.hpp"
#include "opencv2/slideio/imagetools.hpp"
#include <set>

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

CZIScene::CZIScene() : m_slide(nullptr), m_numZSlices(1), m_numTFrames(1)
{
}

std::string CZIScene::getFilePath() const
{
    return m_filePath;
}

cv::Rect CZIScene::getRect() const
{
    return m_sceneRect;
}

int CZIScene::getNumChannels() const
{
    return static_cast<int>(m_componentInfos.size());
}

int CZIScene::getNumZSlices() const
{
    return m_numZSlices;
}

int CZIScene::getNumTFrames() const
{
    return m_numTFrames;
}

double CZIScene::getZSliceResolution() const
{
    return m_slide->getZSliceResolution();
}

double CZIScene::getTFrameResolution() const
{
    return getTFrameResolution();
}

cv::slideio::DataType CZIScene::getChannelDataType(int channel) const
{
    if (channel < 0 || channel >= getNumChannels())
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Invalid channel index: %1%") % channel).str());
    }
    return m_componentInfos[channel].dataType;
}

std::string CZIScene::getChannelName(int channel) const
{
    if (channel < 0 || channel >= getNumChannels())
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Invalid channel index: %1%") % channel).str());
    }
    return m_componentInfos[channel].name;
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
    const std::vector<int>& componentIndices, cv::OutputArray output)
{
    TilerData userData;
    const double zoomX = static_cast<double>(blockSize.width) / static_cast<double>(blockRect.width);
    const double zoomY = static_cast<double>(blockSize.height) / static_cast<double>(blockRect.width);
    const double zoom = std::max(zoomX, zoomY);
    const std::vector<ZoomLevel>& zoomLevels = m_zoomLevels;
    userData.zoomLevelIndex = Tools::findZoomLevel(zoom, static_cast<int>(m_zoomLevels.size()), [&zoomLevels](int index){
        return zoomLevels[index].zoom;
    });
    const double levelZoom = zoomLevels[userData.zoomLevelIndex].zoom;
    cv::Rect zoomLevelRect;
    ImageTools::scaleRect(blockRect, levelZoom, levelZoom, zoomLevelRect);
    userData.relativeZoom = levelZoom / zoom;
    userData.zSliceIndex = 0;
    userData.tFrameIndex = 0;
    TileComposer::composeRect(this, componentIndices, zoomLevelRect, blockSize, output, &userData);
}

std::string CZIScene::getName() const
{
    return m_name;
}

void CZIScene::generateSceneName()
{
    m_name = (boost::format("%1%(s:%2% i:%3% v:%4% h:%5% r:%6% b:%7%)")
        % m_slide->getTitle()
        % m_sceneParams.sceneIndex
        % m_sceneParams.illuminationIndex
        % m_sceneParams.viewIndex
        % m_sceneParams.hPhaseIndex
        % m_sceneParams.rotationIndex
        % m_sceneParams.bAccusitionIndex).str();
}

void CZIScene::computeSceneRect()
{
    // compute scene rectangle
    const CZIScene::ZoomLevel& zoomLevelMax = CZIScene::getBaseZoomLevel();
    m_sceneRect = { 0,0,0,0 };
    const Tiles& tiles = zoomLevelMax.tiles;
    const CZISubBlocks& tileBlocks = zoomLevelMax.blocks;
    for(size_t index = 0; index<tiles.size(); index++)
    {
        int blockIndex = tiles[index].blockIndices[0];
        const CZISubBlock& block = tileBlocks[blockIndex];
        const cv::Rect& tileRect = block.rect();
        m_sceneRect |= tileRect;
    }
}

void CZIScene::computeSceneTiles()
{
    // combine zoom level blocks in tiles
    for(auto& zoomLevel: m_zoomLevels)
    {
        combineBlockInTiles(zoomLevel);
    }
}

void CZIScene::compute4DParameters()
{
    const CZIScene::ZoomLevel& zoomLevelMax = CZIScene::getBaseZoomLevel();
    auto blocks = zoomLevelMax.blocks;
    int firstZSlice(0), lastZSlice(0), firstTFrame(0), lastTFrame(0);
    for(size_t blockIndex=0; blockIndex<blocks.size(); ++blockIndex)
    {
        const slideio::CZISubBlock& block = blocks[blockIndex];
        if(blockIndex==0)
        {
            firstTFrame = block.firstTFrame();
            lastTFrame = block.lastTFrame();
            firstZSlice = block.firstZSlice();
            lastZSlice = block.lastZSlice();
        }
        firstTFrame = std::min(firstTFrame, block.firstTFrame());
        lastTFrame = std::max(lastTFrame, block.lastTFrame());
        firstZSlice = std::min(firstZSlice, block.firstZSlice());
        lastZSlice = std::max(lastZSlice, block.lastZSlice());
    }
    if(firstZSlice>0 || firstTFrame>0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Unexpected 4D configuration: Z:(%1%-%2%) Time:(%3%-%4%)")
                % firstZSlice
                % lastZSlice
                % firstTFrame
                % lastTFrame).str());
    }
    m_numZSlices = lastZSlice + 1;
    m_numTFrames = lastTFrame + 1;
}

const CZIScene::ZoomLevel& CZIScene::getBaseZoomLevel() const
{
    const ZoomLevel& zoomLevelMax = m_zoomLevels.front();
    if(abs(zoomLevelMax.zoom-1)>1.e-4)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: unexpected value for max zoom level. Expected: 1, received: %1%") %
                zoomLevelMax.zoom).str());
    }
    return zoomLevelMax;
}


void CZIScene::init(uint64_t sceneId, SceneParams& sceneParams, const std::string& filePath, const CZISubBlocks& blocks, CZISlide* slide)
{
    m_sceneParams = sceneParams;
    m_slide = slide;
    m_id = sceneId;
    // separate blocks by zoom levels and detect count of channels and channel data type
    m_filePath = filePath;
    std::map<double, int, double_less> zoomLevelIndices;
    std::map<int, int> channelPixelType;
    for(const auto& block : blocks)
    {
        double zoom = block.zoom();
        int zoomLevelIndex = 0;
        auto itIndex = zoomLevelIndices.find(zoom);
        if(itIndex==zoomLevelIndices.end())
        {
            zoomLevelIndex = static_cast<int>(zoomLevelIndices.size());
            zoomLevelIndices[zoom] = zoomLevelIndex;
            m_zoomLevels.emplace_back();
            m_zoomLevels.back().zoom = zoom;
        }
        for(int channelIndex=block.firstChannel(); channelIndex<=block.lastChannel(); channelIndex++)
        {
            channelPixelType[channelIndex] = block.cziPixelType();
        }
        m_zoomLevels[zoomLevelIndex].blocks.push_back(block);
    }
    setupComponents(channelPixelType);
    // sort zoom levels in ascending order
    std::sort(m_zoomLevels.begin(), m_zoomLevels.end(), [](const ZoomLevel& left, const ZoomLevel& right)
    {
        return (abs(left.zoom - right.zoom) > DOUBLE_EPSILON) && (left.zoom > right.zoom);
    });
    computeSceneTiles();
    computeSceneRect();
    compute4DParameters();
    generateSceneName();
}

int CZIScene::getTileCount(void* userData)
{
    TilerData* tilerData = reinterpret_cast<TilerData*>(userData);
    const int zoomLevelIndex = tilerData->zoomLevelIndex;
    const ZoomLevel& zoomLevel = m_zoomLevels[zoomLevelIndex];
    return static_cast<int>(zoomLevel.tiles.size());
}

bool CZIScene::getTileRect(int tileIndex, cv::Rect& tileRect, void* userData)
{
    auto tilerData = reinterpret_cast<TilerData*>(userData);
    const int zoomLevelIndex = tilerData->zoomLevelIndex;
    const ZoomLevel& zoomLevel = m_zoomLevels[zoomLevelIndex];
    const Tiles& tiles = zoomLevel.tiles;
    const int firstBlockIndex = tiles[tileIndex].blockIndices[0];
    const CZISubBlock& firstBlock = zoomLevel.blocks[firstBlockIndex];
    tileRect = firstBlock.rect();
    tileRect.x = static_cast<int>(std::ceil(static_cast<double>(tileRect.x)* zoomLevel.zoom));
    tileRect.y = static_cast<int>(std::ceil(static_cast<double>(tileRect.y)* zoomLevel.zoom));
    return true;
}


int CZIScene::findBlockIndex(const Tile& tile, const CZISubBlocks& blocks, int channelIndex, int zSliceIndex, int tFrameIndex) const
{
    for(const auto& blockIndex : tile.blockIndices)
    {
        const CZISubBlock& block = blocks[blockIndex];
        if( channelIndex >= block.firstChannel() &&
            channelIndex <= block.lastChannel() && 
            zSliceIndex >= block.firstZSlice() &&
            zSliceIndex <= block.lastZSlice() &&
            tFrameIndex <= block.firstTFrame() &&
            tFrameIndex >= block.lastTFrame())
        {
            // block found
            return blockIndex;
        }
    }
    throw std::runtime_error(
        (boost::format("CZIImageDriver: Cannot find sub-block (c:%1%, z:%2%, t:%3%) of file %4%") 
            % channelIndex 
            % zSliceIndex 
            % tFrameIndex 
            % m_filePath).str()
    );
}

const CZIScene::Tile& CZIScene::getTile(const TilerData* tilerData, int tileIndex) const
{
    const int zoomLevelIndex = tilerData->zoomLevelIndex;
    const ZoomLevel& zoomLevel = m_zoomLevels[zoomLevelIndex];
    const Tiles& tiles = zoomLevel.tiles;
    const Tile& tile = tiles[tileIndex];
    return tile;
}

const CZISubBlocks& CZIScene::getBlocks(const TilerData* tilerData) const
{
    const int zoomLevelIndex = tilerData->zoomLevelIndex;
    const ZoomLevel& zoomLevel = m_zoomLevels[zoomLevelIndex];
    return zoomLevel.blocks;
}


bool CZIScene::blockHasData(const CZISubBlock& block, const std::vector<int>& componentIndices, const TilerData* tilerData)
{
    for(int component : componentIndices)
    {
        const int channel = m_componentToChannelIndex[component].first;
        if(block.isInBlock(channel,
            tilerData->zSliceIndex,
            tilerData->tFrameIndex,
            m_sceneParams.rotationIndex,
            m_sceneParams.sceneIndex,
            m_sceneParams.illuminationIndex,
            m_sceneParams.bAccusitionIndex,
            m_sceneParams.hPhaseIndex,
            m_sceneParams.viewIndex))
        {
            return true;
        }
    }
    return false;
}

std::vector<uint8_t> CZIScene::decodeData(const CZISubBlock& block, const std::vector<unsigned char>& encodedData)
{
    if(block.compression()==CZISubBlock::Uncompressed)
    {
        return encodedData;
    }
    throw std::runtime_error(
        (boost::format("CZIImageDriver: Unsupported compression %1%") % static_cast<int>(block.compression())).str()
    );
}

void CZIScene::unpackChannels(const CZISubBlock& block, const std::vector<int>& componentIndices, 
    const std::vector<unsigned char>& blockData, const TilerData* tilerData, 
    std::vector<cv::Mat>& componentRasters)
{
    for(int index=0; index<componentIndices.size(); ++index)
    {
        const int componentIndex = componentIndices[index];
        const std::pair<int,int> componentChannelInfo = m_componentToChannelIndex[componentIndex];
        const int channelIndex = componentChannelInfo.first;
        const int channelComponent = componentChannelInfo.second;
        const int64_t channelOffset = block.computeDataOffset(channelIndex,
            tilerData->zSliceIndex,
            tilerData->tFrameIndex,
            m_sceneParams.rotationIndex,
            m_sceneParams.sceneIndex,
            m_sceneParams.illuminationIndex,
            m_sceneParams.bAccusitionIndex,
            m_sceneParams.hPhaseIndex,
            m_sceneParams.viewIndex);

        if(channelOffset<0)
            continue;

        const uint8_t* channelData = blockData.data() + channelOffset;
        const SceneChannelInfo& channelInfo = m_channelInfos[componentIndex];
        const int channelSize = block.planeSize();
        const int cvPixelType = static_cast<int>(block.dataType());
        const cv::Size rasterSize = block.rect().size();

        if(channelInfo.numComponents==1)
        {
            componentRasters[index].create(rasterSize, CV_MAKETYPE(cvPixelType, 1));
            uint8_t* trg = componentRasters[index].data;
            std::memcpy(trg, channelData, channelSize);
        }
        else
        {
            cv::Mat channelRaster(rasterSize, CV_MAKETYPE(cvPixelType, channelInfo.numComponents));
            extractChannel(channelRaster, componentRasters[index], channelComponent);
        }
    }
}

bool CZIScene::readTile(int tileIndex, const std::vector<int>& orgComponentIndices, cv::OutputArray tileRaster,
                        void* userData)
{
    const TilerData* tilerData = reinterpret_cast<TilerData*>(userData);
    const Tile& tile = getTile(tilerData, tileIndex);
    const CZISubBlocks& blocks = getBlocks(tilerData);
    std::vector<uint8_t> data;
    const int numChannels = getNumChannels();
    const std::vector<int> componentIndices = Tools::completeChannelList(orgComponentIndices, numChannels);
    const int firstComponent = componentIndices[0];
    const int cvDataType = static_cast<int>(getChannelDataType(firstComponent));
    cv::Rect tileRect;
    getTileRect(tileIndex, tileRect, userData);
    tileRaster.create(tileRect.size(), CV_MAKETYPE(cvDataType, numChannels));
    std::vector<cv::Mat> channelRasters(componentIndices.size());
    for(int index: tile.blockIndices)
    {
        const CZISubBlock& block = blocks[index];
        if(blockHasData(block, componentIndices, tilerData))
        {
            uint64_t pos = block.dataPosition();
            uint64_t size = block.dataSize();
            m_slide->readBlock(pos, size, data);
            std::vector<uint8_t> rasterData = decodeData(block, data);
            unpackChannels(block, componentIndices, rasterData, tilerData, channelRasters);
        }
    }
    if(channelRasters.size()==1)
    {
        channelRasters[0].copyTo(tileRaster);
    }
    else
    {
        cv::merge(channelRasters, tileRaster);
    }
    return true;
}


void CZIScene::combineBlockInTiles(ZoomLevel& zoomLevel)
{
    std::map<uint64_t, int> coordsToIndex;
    Tiles& tiles = zoomLevel.tiles;
    for(int blockIndex=0; blockIndex < static_cast<int>(zoomLevel.blocks.size()); blockIndex++)
    {
        const CZISubBlock& block = zoomLevel.blocks[blockIndex];
        const cv::Rect& rectBlock = block.rect();
        uint64_t key = (static_cast<uint64_t>(rectBlock.x) << 32) | rectBlock.y;
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

void CZIScene::channelComponentInfo(CZIDataType channelCZIDataType, DataType& componentType, int& numComponents,
    int& pixelSize)
{
    switch (channelCZIDataType)
    {
    case Gray8:
        componentType = DataType::DT_Byte;
        numComponents = 1;
        pixelSize = 1;
        break;
    case Gray16:
        componentType = DataType::DT_UInt16;
        numComponents = 1;
        pixelSize = 2;
        break;
    case Gray32Float:
        componentType = DataType::DT_Float32;
        numComponents = 1;
        pixelSize = 4;
        break;
    case Bgr24:
        componentType = DataType::DT_Byte;
        pixelSize = 3;
        numComponents = 3;
        break;
    case Bgr48:
        componentType = DataType::DT_UInt16;
        pixelSize = 6;
        numComponents = 3;
        break;
    case Bgr96Float:
        componentType = DataType::DT_Float32;
        pixelSize = 12;
        numComponents = 3;
        break;
    case Bgra32:
        componentType = DataType::DT_Byte;
        pixelSize = 4;
        numComponents = 4;
        break;
    case Gray32:
        componentType = DataType::DT_Int32;
        pixelSize = 4;
        numComponents = 1;
        break;
    case Gray64:
        componentType = DataType::DT_Float64;
        pixelSize = 8;
        numComponents = 1;
        break;
    case Bgr192ComplexFloat:
    case Gray64ComplexFloat:
    default:
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Unsupported data type: %1%") % channelCZIDataType).str());
    }
}

void CZIScene::setupComponents(const std::map<int, int>& channelPixelType)
{
    int sceneComponentIndex = 0;
    const CZIChannelInfos& fileChannelInfo = m_slide->getChannelInfo();
    m_channelInfos.resize(fileChannelInfo.size());
    for(int channelIndex=0; channelIndex < static_cast<int>(fileChannelInfo.size()); ++channelIndex)
    {
        m_channelInfos[channelIndex].name = fileChannelInfo[channelIndex].id;
    }
    int componentIndex = 0;
    for(const auto& channel : channelPixelType)
    {
        int channelIndex = channel.first;
        SceneChannelInfo& channelInfo = m_channelInfos[channelIndex];
        CZIDataType channelCZIDataType = static_cast<CZIDataType>(channel.second);
        channelComponentInfo(channelCZIDataType, channelInfo.componentType, channelInfo.numComponents, channelInfo.pixelSize);
        channelInfo.firstComponent = componentIndex;
        componentIndex += channelInfo.numComponents;
        for(int blockComponentIndex=0; blockComponentIndex< channelInfo.numComponents; ++blockComponentIndex, ++sceneComponentIndex)
        {
            m_componentToChannelIndex[sceneComponentIndex] = std::pair<int,int>(channelIndex, blockComponentIndex);
            m_componentInfos.emplace_back();
            auto& componentInfo = m_componentInfos.back();
            componentInfo.dataType = channelInfo.componentType;
            std::string channelName = fileChannelInfo[channelIndex].name;
            if(channelName.empty())
                channelName = fileChannelInfo[channelIndex].id;
            if(channelIndex<static_cast<int>(fileChannelInfo.size()))
            {
                if(channelInfo.numComponents ==1)
                {
                    componentInfo.name = channelName;
                }
                else
                {
                    componentInfo.name = (boost::format("%1%:%2%") % channelName % (blockComponentIndex+1)).str();
                }
            }
        }
    }
}

uint64_t CZIScene::sceneIdFromDims(int s, int i, int v, int h, int r, int b)
{
    const uint64_t values[] = { (uint64_t)s, (uint64_t)i , (uint64_t)v, (uint64_t)h, (uint64_t)r, (uint64_t)b };
    const uint64_t digitsPerValue = 1000;
    uint64_t sceneId = 0;
    uint64_t mult = 1;
    for (int val = 0; val < 6; ++val)
    {
        sceneId += values[val] * mult;
        mult *= digitsPerValue;
    }
    return sceneId;
}


uint64_t CZIScene::sceneIdFromDims(const std::vector<Dimension>& dims)
{
    int s(0), i(0), v(0), h(0), r(0), b(0);
    for (const auto& dim : dims)
    {
        switch (dim.type)
        {
        case 'S': s = dim.start; break;
        case 'I': i = dim.start; break;
        case 'V': v = dim.start; break;
        case 'H': h = dim.start; break;
        case 'R': r = dim.start; break;
        case 'B': b = dim.start; break;
        default: break;
        }
    }
    return sceneIdFromDims(s, i, v, h, r, b);
}

inline int getDimensionValue(const std::map<char, int>& dimensionIndices, char dim, std::vector<int>& dimensionValues)
{
    int val = 0;
    auto it = dimensionIndices.find(dim);
    if(it!=dimensionIndices.end())
    {
        int index = it->second;
        val = dimensionValues[index];
    }
    return val;
}

static void extractSceneIds(const std::vector<Dimension>& dims, const std::map<char, int>& dimensionIndices,
    std::vector<int>& dimensionValues, int curDim, std::set<uint64_t>& sceneIds)
{
    const auto& dim = dims[curDim];
    const int startIndex = dim.start;
    const int stopIndex = startIndex + dim.size;
    const int nextDim = curDim + 1;
    for(int index=startIndex; index<stopIndex; ++index)
    {
        dimensionValues[curDim] = index;
        if(nextDim < static_cast<int>(dims.size()))
        {
            extractSceneIds(dims, dimensionIndices, dimensionValues, nextDim, sceneIds);
        }
        else
        {
            const auto s = getDimensionValue(dimensionIndices, 'S', dimensionValues);
            const auto i = getDimensionValue(dimensionIndices, 'I', dimensionValues);
            const auto v = getDimensionValue(dimensionIndices, 'V', dimensionValues);
            const auto h = getDimensionValue(dimensionIndices, 'H', dimensionValues);
            const auto r = getDimensionValue(dimensionIndices, 'R', dimensionValues);
            const auto b = getDimensionValue(dimensionIndices, 'B', dimensionValues);

            uint64_t sceneId = CZIScene::sceneIdFromDims(s,i,v,h,r,b);
            sceneIds.insert(sceneId);
        }
    }
}

void CZIScene::sceneIdsFromDims(const std::vector<Dimension>& dims, std::vector<uint64_t>& ids)
{
    std::map<char, int> dimensionIndices;
    for(int dim=0; dim < static_cast<int>(dims.size()); ++dim)
    {
        dimensionIndices[dims[dim].type] = dim;
    }
    std::set<uint64_t> sceneIdset;
    std::vector<int> indices(dims.size());
    extractSceneIds(dims, dimensionIndices, indices, 0, sceneIdset);
    std::copy(sceneIdset.begin(), sceneIdset.end(), std::back_inserter(ids));
}

uint64_t CZIScene::sceneIdFromDims(const SceneParams& params)
{
    return sceneIdFromDims(
        params.sceneIndex,
        params.illuminationIndex,
        params.viewIndex,
        params.hPhaseIndex,
        params.rotationIndex,
        params.bAccusitionIndex);
}

void CZIScene::dimsFromSceneId(uint64_t sceneId, int& s, int& i, int& v, int& h, int& r, int& b)
{
    uint64_t values[6] = { 0 };
    const uint64_t digitsPerValue = 1000;
    uint64_t mult1 = 1000;
    uint64_t mult2 = 1;
    for (int val = 0; val < 6; val++)
    {
        values[val] = (sceneId % mult1) / mult2;
        mult1 *= digitsPerValue;
        mult2 *= digitsPerValue;
    }
    s = (int)values[0];
    i = (int)values[1];
    v = (int)values[2];
    h = (int)values[3];
    r = (int)values[4];
    b = (int)values[5];
}

void CZIScene::dimsFromSceneId(uint64_t sceneId, SceneParams& params)
{
    dimsFromSceneId(sceneId,
        params.sceneIndex,
        params.illuminationIndex,
        params.viewIndex,
        params.hPhaseIndex,
        params.rotationIndex,
        params.bAccusitionIndex);
}
