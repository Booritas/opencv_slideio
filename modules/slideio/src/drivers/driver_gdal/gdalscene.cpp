#include "opencv2/slideio/gdalscene.hpp"
#include "opencv2/slideio.hpp"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace cv;

slideio::GDALScene::GDALScene(const std::string& path) : m_hFile(nullptr)
{
    m_hFile = openFile(path);
    m_filePath = path;
}

slideio::GDALScene::GDALScene(GDALDatasetH ds, const std::string& path) : m_hFile(ds), m_filePath(path)
{
}

slideio::GDALScene::~GDALScene()
{
    closeFile(m_hFile);
    m_hFile = nullptr;
}

std::string slideio::GDALScene::getScenePath() const
{
    return m_filePath;
}


int slideio::GDALScene::getNumChannels() const
{
    if(m_hFile==nullptr)
        throw std::runtime_error("GDALDriver: Invalid file header by channel number query");
    int channels = GDALGetRasterCount(m_hFile);
    return channels;
}

slideio::DataType slideio::GDALScene::getChannelDataType(int channel) const
{
    if(m_hFile==nullptr)
        throw std::runtime_error("GDALDriver: Invalid file header by data type query");
    GDALRasterBandH hBand = GDALGetRasterBand(m_hFile, channel+1);
    if(hBand==nullptr)
        throw std::runtime_error("GDALDriver:  Cannot get raster band by query of data type");
    const GDALDataType dt = GDALGetRasterDataType(hBand);
    return dataTypeFromGDALDataType(dt);
}

slideio::Resolution  slideio::GDALScene::getResolution() const
{
    double adfGeoTransform[6];
    slideio::Resolution res = {0,0};
    if(GDALGetGeoTransform(m_hFile, adfGeoTransform ) == CE_None )
    {
        res.x = adfGeoTransform[1];
        res.y = adfGeoTransform[5];
    }
    return res;
}

double slideio::GDALScene::getMagnification() const
{
    return 0;
}

GDALDatasetH slideio::GDALScene::openFile(const std::string& filePath)
{
    namespace fs = boost::filesystem;
    if(!fs::exists(filePath)){
        throw std::runtime_error(std::string("File does not exist:") + filePath);
    }
    GDALDatasetH hfile = GDALOpen(filePath.c_str(), GA_ReadOnly);
    if(hfile==nullptr){
        throw std::runtime_error(std::string("Cannot open file with GDAL driver:") + filePath);
    }
    return hfile;
}

inline void slideio::GDALScene::closeFile(GDALDatasetH hfile)
{
    if(hfile!=nullptr)
        GDALClose(hfile);
}

slideio::DataType slideio::GDALScene::dataTypeFromGDALDataType(GDALDataType dt)
{
    switch(dt)
    {
    case GDT_Unknown:
        return slideio::DataType::DT_Unknown;
    case GDT_Byte:
        return slideio::DataType::DT_Byte;
    case GDT_UInt16:
        return slideio::DataType::DT_UInt16;
    case GDT_Int16:
        return slideio::DataType::DT_Int16;
    case GDT_Int32:
        return slideio::DataType::DT_Int32;
    case GDT_Float32:
        return slideio::DataType::DT_Float32;
    case GDT_Float64:
        return slideio::DataType::DT_Float64;
    default:
        return slideio::DataType::DT_Unknown;
    }

}


std::string slideio::GDALScene::getName() const
{
    return std::string();
}

cv::Rect slideio::GDALScene::getSceneRect() const
{
    if (m_hFile == nullptr)
        throw std::runtime_error("GDALDriver: Invalid file header by scene size query");
    cv::Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = GDALGetRasterXSize(m_hFile);
    rect.height = GDALGetRasterYSize(m_hFile);
    return rect;
}

void slideio::GDALScene::readBlock(const cv::Rect& blockRect, const cv::Size& blockSize, const std::vector<int>& channelIndices_, cv::OutputArray output)
{
    if(m_hFile==nullptr)
        throw std::runtime_error("GDALDriver: Invalid file header by raster reading operation");
    const int numChannels = GDALGetRasterCount(m_hFile);
    const cv::Size imageSize = { GDALGetRasterXSize(m_hFile),GDALGetRasterYSize(m_hFile) };
    auto channelIndices = channelIndices_;
    if(channelIndices.empty())
    {
        channelIndices.resize(numChannels);
        for (int channelIndex = 0; channelIndex < numChannels; ++channelIndex)
        {
            channelIndices[channelIndex] = channelIndex;
        }
    }
    std::vector<cv::Mat> channelRasters;
    channelRasters.reserve(channelIndices.size());
    for (const auto& channelIndex : channelIndices)
    {
        GDALRasterBandH hBand = GDALGetRasterBand(m_hFile, channelIndex + 1);
        if (hBand == nullptr)
            throw std::runtime_error(
            (boost::format("Cannot open raster band from: %1%") % m_filePath).str());
        const GDALDataType dt = GDALGetRasterDataType(hBand);
        const DataType dataType = dataTypeFromGDALDataType(dt);
        if(!isValidDataType(dataType))
        {
            throw std::runtime_error(
                (boost::format("Unknown data type %1% of channel %2% of file %3%") % dt % channelIndex % m_filePath).str());
        }
        const int cvDt = toOpencvType(dataType);
        cv::Mat channelRaster; 
        int channelType = CV_MAKETYPE(cvDt, 1);
        channelRaster.create(blockSize, channelType);
        CPLErr err = GDALRasterIO(hBand, GF_Read,
            blockRect.x, blockRect.y,
            blockRect.width, blockRect.height,
            channelRaster.data,
            blockSize.width, blockSize.height,
            GDALGetRasterDataType(hBand), 0, 0);
        channelRasters.push_back(channelRaster);
        if (err != CE_None)
            throw std::runtime_error(
            (boost::format("Cannot read raster band %1% from %2%") % channelIndex % m_filePath).str());
    }
    if(channelRasters.size()>1)
    {
        cv::merge(channelRasters, output);
    }
    else if(channelRasters.size()==1)
    {
        channelRasters[0].copyTo(output);
    }
}