#include "opencv2/slideio/svstiledscene.hpp"
#include "opencv2/slideio/tifftools.hpp"
#include "opencv2/slideio/svstools.hpp"
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/slideio/svsscene.hpp"

using namespace cv::slideio;

SVSTiledScene::SVSTiledScene(const std::string& filePath,
    const std::string& name,
    std::vector<TiffDirectory> dirs, TIFF* hfile):
    slideio::SVSScene(filePath, name),
        m_directories(dirs),
        m_dataType(slideio::DataType::DT_Unknown),
        m_hFile(hfile)
{
    auto& dir = m_directories[0];
    m_dataType = TiffTools::dataTypeFromTIFFDataType(dir.dataType);

    if(m_dataType==DataType::DT_None || m_dataType==DataType::DT_Unknown)
    {
        switch(dir.bitsPerSample)
        {
            case 8:
                m_dataType = DataType::DT_Byte;
                dir.dataType = TIFF_BYTE;
            break;
            case 16:
                m_dataType = DataType::DT_UInt16;
                dir.dataType = TIFF_SHORT;
            break;
            default:
                m_dataType = DataType::DT_Unknown;
        }
    }
    m_magnification = SVSTools::extractMagnifiation(dir.description);
}

cv::Rect SVSTiledScene::getSceneRect() const
{
    cv::Rect rect = { 0,0,  m_directories[0].width,  m_directories[0].height };
    return rect;
}

int SVSTiledScene::getNumChannels() const
{
    const auto& dir = m_directories[0];
    return dir.channels;
}

DataType SVSTiledScene::getChannelDataType(int) const
{
    return m_dataType;
}

Resolution SVSTiledScene::getResolution() const
{
    const auto& dir = m_directories[0];
    return dir.res;
}

double SVSTiledScene::getMagnification() const
{
    return m_magnification;
}

void SVSTiledScene::readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
    const std::vector<int>& channelIndices, cv::OutputArray output)
{
    if (m_hFile == nullptr)
        throw std::runtime_error("SVSDriver: Invalid file header by raster reading operation");
    double zoomX = static_cast<double>(blockSize.width) / static_cast<double>(blockRect.width);
    double zoomY = static_cast<double>(blockSize.height) / static_cast<double>(blockRect.height);
    double zoom = std::max(zoomX, zoomY);
    const slideio::TiffDirectory& dir = findZoomDirectory(zoom);
    double zoomDirX = static_cast<double>(dir.width) / static_cast<double>(m_directories[0].width);
    double zoomDirY = static_cast<double>(dir.height) / static_cast<double>(m_directories[0].height);
    cv::Rect resizedBlock;
    ImageTools::scaleRect(blockRect, zoomDirX, zoomDirY, resizedBlock);
    TileComposer::composeRect(this, channelIndices, resizedBlock, blockSize, output, (void*)&dir);
}

const TiffDirectory& SVSTiledScene::findZoomDirectory(double zoom) const
{
    const cv::Rect sceneRect = getSceneRect();
    const TiffDirectory& baseDir = m_directories[0];
    double baseZoom = static_cast<double>(baseDir.width) / static_cast<double>(sceneRect.width);
    if(zoom>= baseZoom)
    {
        return m_directories[0];
    }
    int dirGoodIndex = -1;
    double lastZoom = baseZoom;
    for(int dirIndex=1; dirIndex<m_directories.size(); dirIndex++)
    {
        const TiffDirectory& dir = m_directories[dirIndex];
        double currentZoom = static_cast<double>(dir.width) / static_cast<double>(sceneRect.width);
        double absDif = std::abs(currentZoom - zoom);
        double relDif = absDif / currentZoom;
        if(relDif < 0.01)
        {
            dirGoodIndex = dirIndex;
            break;
        }
        if(zoom<=lastZoom && zoom>currentZoom)
        {
            dirGoodIndex = dirIndex-1;
            break;
        }
        lastZoom = currentZoom;
    }
    if(dirGoodIndex<0)
    {
        dirGoodIndex = static_cast<int>(m_directories.size()) - 1;
    }
    return  m_directories[dirGoodIndex];
}

int SVSTiledScene::getTileCount(void* userData)
{
    const TiffDirectory* dir = (const TiffDirectory*)userData;
    int tilesX = (dir->width-1)/dir->tileWidth + 1;
    int tilesY = (dir->height-1)/dir->tileHeight + 1;
    return tilesX * tilesY;
}

bool SVSTiledScene::getTileRect(int tileIndex, cv::Rect& tileRect, void* userData)
{
    const TiffDirectory* dir = (const TiffDirectory*)userData;
    const int tilesX = (dir->width - 1) / dir->tileWidth + 1;
    const int tilesY = (dir->height - 1) / dir->tileHeight + 1;
    const int tileY = tileIndex / tilesX;
    const int tileX = tileIndex % tilesX;
    tileRect.x = tileX * dir->tileWidth;
    tileRect.y = tileY * dir->tileHeight;
    tileRect.width = dir->tileWidth;
    tileRect.height = dir->tileHeight;
    return true;
}

bool SVSTiledScene::readTile(int tileIndex, const std::vector<int>& channelIndices, cv::OutputArray tileRaster,
    void* userData)
{
    const TiffDirectory* dir = (const TiffDirectory*)userData;
    TiffTools::readTile(m_hFile, *dir, tileIndex, channelIndices, tileRaster);
    return true;
}

