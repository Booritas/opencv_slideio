// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/cziscene.hpp"
#include "czistructs.hpp"

#include <boost/format.hpp>
#include <tinyxml2.h>

using namespace cv::slideio;
using namespace tinyxml2;

static char SID_FILES[] = "ZISRAWFILE";
static char SID_METADATA[] = "ZISRAWMETADATA";
static char SID_DIRECTORY[] = "ZISRAWDIRECTORY";

//-------------------------------------------------------
// Static helper functions for parsing of the metadata
// ------------------------------------------------------
static int xmlChildNodeTextToInt(const XMLNode* xmlParent, const char* childName, int defaultValue=-1)
{
    if(xmlParent==nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
    const XMLElement* xmlChild = xmlParent->FirstChildElement(childName);
    int value = defaultValue;
    if(xmlChild!=nullptr)
        value = xmlChild->IntText(defaultValue);
    return value;
}

static const XMLElement* getXmlElementByPath(const XMLNode* parent, const std::vector<std::string>& path)
{
    const XMLElement* xmlCurrentElement = nullptr;
    const XMLNode* xmlCurrentNode = parent;
    for(const auto& tag: path)
    {
        xmlCurrentElement = xmlCurrentNode->FirstChildElement(tag.c_str());
        if(xmlCurrentElement==nullptr)
        {
            throw std::runtime_error(
                (boost::format("CZIImageDriver: Invalid xml. Cannot find node %1%.") % tag).str());
        }
        xmlCurrentNode = xmlCurrentElement;
    }
    return xmlCurrentElement;
}

CZIScene::CZIScene() :
            m_slideXs(0),
            m_slideYs(0),
            m_slideZs(0),
            m_slideTs(0),
            m_slideRs(0),
            m_slideIs(0),
            m_slideSs(0),
            m_slideHs(0),
            m_slideMs(0),
            m_slideBs(0),
            m_slideVs(0),
            m_magnification(0),
            m_resX(0),
            m_resY(0),
            m_resZ(0)
{
}

std::string CZIScene::getScenePath() const
{
    return m_filePath;
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
    return Resolution(m_resX, m_resY);
}

double CZIScene::getMagnification() const
{
    return m_magnification;
}

void CZIScene::readResampledBlockChannels(const cv::Rect& blockRect, const cv::Size& blockSize,
    const std::vector<int>& channelIndices, cv::OutputArray output)
{
}

std::string CZIScene::getName() const
{
    return "";
}

void CZIScene::init(const std::string& filePath)
{
    m_filePath = filePath;
    // read file header
    m_fileStream.exceptions(std::ios::failbit | std::ios::badbit);
    m_fileStream.open(m_filePath.c_str(), std::ifstream::in|std::ifstream::binary);
    readFileHeader();
    readMetadata();
    readDirectory();
}

void CZIScene::parseMagnification(XMLNode* root)
{
    const std::vector<std::string> magnificationPath = {
        "ImageDocument","Metadata","Information", "Instrument",
        "Objectives", "Objective", "NominalMagnification"
    };
    const XMLElement *xmlMagnification = getXmlElementByPath(root, magnificationPath);
    m_magnification = xmlMagnification->FloatText(20.);
}

void CZIScene::parseMetadataXmL(const char* xmlString, size_t dataSize)
{
    XMLDocument doc;
    XMLError error = doc.Parse(xmlString, dataSize);
    if(error!= XML_SUCCESS)
    {
        throw std::runtime_error("CZIImageDriver: Error parsing metadata xml");
    }
    //doc.SaveFile(R"(c:\Temp\czi.xml)");
    parseSizes(&doc);
    parseMagnification(&doc);
    parseResolutions(&doc);
    parseChannels(&doc);
}

void CZIScene::parseChannels(XMLNode* root)
{
    const std::vector<std::string> imagePath = {
        "ImageDocument","Metadata","Information", "Image",
        "Dimensions", "Channels"
    };
    const XMLElement *xmlChannels = getXmlElementByPath(root, imagePath);
    if(xmlChannels==nullptr)
    {
        throw std::runtime_error("CZIImageDriver: Invalid xml: no channel information");
    }
    for(auto xmlChannel = xmlChannels->FirstChildElement("Channel");
        xmlChannel!=nullptr; xmlChannel = xmlChannel->NextSiblingElement())
    {
        const char* name = xmlChannel->Name();
        if(name && strcmp(name,"Channel")==0)
        {
            Channel channel;
            const char* id = xmlChannel->Attribute("Id");
            if(id)
            {
                channel.id = id;
            }
            const XMLElement* pixelType = xmlChannel->FirstChildElement("PixelType");
            if(pixelType==nullptr)
            {
                throw std::runtime_error("CZIImageDriver: invalid xml: no pixel information");
            }
            const char * pixelTypeName = pixelType->GetText();
            if(pixelTypeName==nullptr)
            {
                throw std::runtime_error("CZIImageDriver: invalid xml: no pixel information");
            }
            if(strcmp(pixelTypeName,"Gray8")==0)
            {
                channel.dataType = DataType::DT_Byte;
                channel.componentCount = 1;
            }
            else if(strcmp(pixelTypeName,"Gray16")==0)
            {
                channel.dataType = DataType::DT_Int16;
                channel.componentCount = 1;
            }
            else if(strcmp(pixelTypeName,"Bgr24")==0)
            {
                channel.dataType = DataType::DT_Byte;
                channel.componentCount = 3;
            }
            else if(strcmp(pixelTypeName,"Bgra32")==0)
            {
                channel.dataType = DataType::DT_Byte;
                channel.componentCount = 4;
            }
            else if(strcmp(pixelTypeName,"Gray32Float")==0)
            {
                channel.dataType = DataType::DT_Float32;
                channel.componentCount = 1;
            }
            else
            {
                throw std::runtime_error(
                    (boost::format("CZIImageDriver: unsupported pixel type %1%") % pixelTypeName).str());
            }
        }
    }
}

void CZIScene::readMetadata()
{
    // position stream pointer to metadata segment
    m_fileStream.seekg(m_metadataPosition, std::ios_base::beg);
    // read segment header
    SegmentHeader header;
    m_fileStream.read((char*)&header, sizeof(header));
    if(strncmp(header.SID,SID_METADATA, sizeof(SID_METADATA))!=0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: invalid metadata segment in file %1%.") % m_filePath).str());
    }
    // read metadata header
    MetadataHeader metadataHeader;
    m_fileStream.read((char*)&metadataHeader, sizeof(metadataHeader));
    const int32_t metadataHeaderSize = 256;
    const uint32_t xmlSize = metadataHeader.xmlSize;;
    std::vector<char> xmlString(xmlSize);
    // read metadata xml
    m_fileStream.read(xmlString.data(), xmlSize);
    parseMetadataXmL(xmlString.data(), xmlSize);
}

void CZIScene::readFileHeader()
{
    FileHeader fileHeader;
    SegmentHeader header;
    m_fileStream.read((char*)&header, sizeof(header));
    if(strncmp(header.SID,SID_FILES, sizeof(SID_FILES))!=0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: file %1% is not a CZI file.") % m_filePath).str());
    }
    m_fileStream.read((char*)&fileHeader, sizeof(fileHeader));
    m_directoryPosition = fileHeader.directoryPosition;
    m_metadataPosition = fileHeader.metadataPosition;
}

void CZIScene::readDirectory()
{
    // position stream pointer to the directory segment
    m_fileStream.seekg(m_directoryPosition, std::ios_base::beg);
    // read segment header
    SegmentHeader header;
    m_fileStream.read((char*)&header, sizeof(header));
    if(strncmp(header.SID,SID_DIRECTORY, sizeof(SID_DIRECTORY))!=0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: invalid directory segment of file %1%.") % m_filePath).str());
    }
    DirectoryHeader directoryHeader;
    m_fileStream.read((char*)&directoryHeader, sizeof(directoryHeader));
    std::vector<Block> blocks(directoryHeader.entryCount);
    for(unsigned int entry=0; entry<directoryHeader.entryCount; ++entry)
    {
        Block& block = blocks[entry];
        DirectoryEntryDV entryHeader;
        m_fileStream.read((char*)&entryHeader, sizeof(entryHeader));
        block.filePosition = entryHeader.filePosition;
        block.compression = entryHeader.compression;
        block.filePart = entryHeader.filePart;
        block.pixelType = entryHeader.pixelType;
        block.dimensions.reserve(entryHeader.dimensionCount);
        for(int dim = 0; dim<entryHeader.dimensionCount; ++dim)
        {
            DimensionEntryDV dimEntry;
            m_fileStream.read((char*)&dimEntry, sizeof(dimEntry));
            if(dimEntry.dimension[0]=='X')
            {
                block.x = dimEntry.start;
                block.width = dimEntry.storedSize;
                block.zoom = dimEntry.storedSize/dimEntry.size;
            }
            else if(dimEntry.dimension[0]=='Y')
            {
                block.y = dimEntry.start;
                block.height = dimEntry.storedSize;
            }
            else if(dimEntry.dimension[0]=='S')
            {
                block.scene = dimEntry.start;
                if(dimEntry.size!=1)
                {
                    throw std::runtime_error("CZIImageDriver: unexpected sub-block configuration");
                }
            }
            else
            {
                Dimension dimension;
                dimension.type = dimEntry.dimension[0];
                dimension.start = dimEntry.start;
                dimension.size = dimEntry.size;
                block.dimensions.push_back(dimension);
            }
            
        }
    }

}

void CZIScene::parseResolutions(XMLNode* root)
{
    const std::vector<std::string> scalingItemsPath = {
        "ImageDocument","Metadata","Scaling", "Items"
    };
    // resolutions
    const XMLElement* xmlItems = getXmlElementByPath(root, scalingItemsPath);
    for(auto child = xmlItems->FirstChildElement(); child!=nullptr;
        child = child->NextSiblingElement())
    {
        const char* name = child->Name();
        if(name && strcmp(name,"Distance")==0)
        {
            const char* id = child->Attribute("Id");
            if(id)
            {
                const XMLElement* valueElement = child->FirstChildElement("Value");
                if(valueElement)
                {
                    double value = valueElement->DoubleText(0);
                    if(strcmp("X",id)==0)
                    {
                        m_resX = value;
                    }
                    else if(strcmp("Y",id)==0)
                    {
                        m_resY = value;
                    }
                    else if(strcmp("Z",id)==0)
                    {
                        m_resZ = value;
                    }
                }
            }
        }
    }
}

void CZIScene::parseSizes(tinyxml2::XMLNode* root)
{
    const std::vector<std::string> imagePath = {"ImageDocument","Metadata","Information", "Image"};
    const XMLElement *xmlImage = getXmlElementByPath(root, imagePath);
    m_slideXs = xmlChildNodeTextToInt(xmlImage, "SizeX");
    m_slideYs = xmlChildNodeTextToInt(xmlImage,"SizeY");
    m_slideZs = xmlChildNodeTextToInt(xmlImage,"SizeZ");
    m_slideTs = xmlChildNodeTextToInt(xmlImage,"SizeT");
    m_slideRs = xmlChildNodeTextToInt(xmlImage,"SizeR");
    m_slideIs = xmlChildNodeTextToInt(xmlImage,"SizeI");
    m_slideSs = xmlChildNodeTextToInt(xmlImage,"SizeS");
    m_slideHs = xmlChildNodeTextToInt(xmlImage,"SizeH");
    m_slideMs = xmlChildNodeTextToInt(xmlImage,"SizeM");
    m_slideBs = xmlChildNodeTextToInt(xmlImage,"SizeB");
    m_slideVs = xmlChildNodeTextToInt(xmlImage,"SizeV");
}
