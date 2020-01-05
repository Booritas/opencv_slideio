// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/czislide.hpp"
#include "opencv2/slideio/cziscene.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <tinyxml2.h>
#include "opencv2/slideio/czistructs.hpp"
#include <set>

using namespace cv::slideio;
using namespace tinyxml2;

static char SID_FILES[] = "ZISRAWFILE";
static char SID_METADATA[] = "ZISRAWMETADATA";
static char SID_DIRECTORY[] = "ZISRAWDIRECTORY";

//-------------------------------------------------------
// Static helper functions for parsing of the metadata
// ------------------------------------------------------
static int xmlChildNodeTextToInt(const XMLNode* xmlParent, const char* childName, int defaultValue = -1)
{
    if (xmlParent == nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
    const XMLElement* xmlChild = xmlParent->FirstChildElement(childName);
    int value = defaultValue;
    if (xmlChild != nullptr)
        value = xmlChild->IntText(defaultValue);
    return value;
}

static const XMLElement* getXmlElementByPath(const XMLNode* parent, const std::vector<std::string>& path)
{
    const XMLElement* xmlCurrentElement = nullptr;
    const XMLNode* xmlCurrentNode = parent;
    for (const auto& tag : path)
    {
        xmlCurrentElement = xmlCurrentNode->FirstChildElement(tag.c_str());
        if (xmlCurrentElement == nullptr)
        {
            return nullptr;
        }
        xmlCurrentNode = xmlCurrentElement;
    }
    return xmlCurrentElement;
}

using namespace cv::slideio;

CZISlide::CZISlide(const std::string& filePath) : m_filePath(filePath)
{
    init();
}

int CZISlide::getNumbScenes() const
{
	return static_cast<int>(m_scenes.size());
}

std::string CZISlide::getFilePath() const
{
	return m_filePath;
}

cv::Ptr<Scene> CZISlide::getScene(int index) const
{
    if(index<0 || index>=getNumbScenes())
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Invalid scene index %1%") % index).str());
    }

	return m_scenes[index];
}


uint64_t CZISlide::sceneIdFromDims(int s, int i, int v, int h, int r, int b)
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

uint64_t CZISlide::sceneIdFromDims(const std::vector<Dimension>& dims)
{
    int s(0), i(0), v(0), h(0), r(0), b(0);
    for(const auto& dim : dims)
    {
        switch(dim.type)
        {
        case 'S': s = dim.start; break;
        case 'I': i = dim.start; break;
        case 'V': v = dim.start; break;
        case 'H': h = dim.start; break;
        case 'R': r = dim.start; break;
        case 'B': b = dim.start; break;
        }
    }
    return sceneIdFromDims(s, i, v, h, r, b);
}

void CZISlide::dimsFromSceneId(uint64_t sceneId, int& s, int& i, int& v, int& h, int& r, int& b)
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

void CZISlide::init()
{
    // read file header
    m_fileStream.exceptions(std::ios::failbit | std::ios::badbit);
    m_fileStream.open(m_filePath.c_str(), std::ifstream::in | std::ifstream::binary);
    readFileHeader();
    readMetadata();
    readDirectory();
}

void CZISlide::parseMagnification(XMLNode* root)
{
    const std::vector<std::string> magnificationPath = {
        "ImageDocument","Metadata","Information", "Instrument",
        "Objectives", "Objective", "NominalMagnification"
    };
    const XMLElement* xmlMagnification = getXmlElementByPath(root, magnificationPath);
    m_magnification = xmlMagnification->FloatText(20.);
}

void CZISlide::parseMetadataXmL(const char* xmlString, size_t dataSize)
{
    XMLDocument doc;
    XMLError error = doc.Parse(xmlString, dataSize);
    if (error != XML_SUCCESS)
    {
        throw std::runtime_error("CZIImageDriver: Error parsing metadata xml");
    }
    const std::vector<std::string> titlePath = {
        "ImageDocument","Metadata","Information", "Document","Title"
    };
    //doc.SaveFile(R"(D:\Temp\czi.xml)");
    const XMLElement* xmlTitle = getXmlElementByPath(&doc, titlePath);
    if(xmlTitle){
        m_title = xmlTitle->GetText();
    }
    parseSizes(&doc);
    parseMagnification(&doc);
    parseResolutions(&doc);
    parseChannels(&doc);
}

void CZISlide::parseChannels(XMLNode* root)
{
    const std::vector<std::string> imagePath = {
        "ImageDocument","Metadata","Information", "Image",
        "Dimensions", "Channels"
    };
    const XMLElement* xmlChannels = getXmlElementByPath(root, imagePath);
    if (xmlChannels == nullptr)
    {
        throw std::runtime_error("CZIImageDriver: Invalid xml: no channel information");
    }
    for (auto xmlChannel = xmlChannels->FirstChildElement("Channel");
        xmlChannel != nullptr; xmlChannel = xmlChannel->NextSiblingElement())
    {
        const char* name = xmlChannel->Name();
        if (name && strcmp(name, "Channel") == 0)
        {
            m_channels.emplace_back();
            CZIChannelInfo& channel = m_channels.back();
            const char* id = xmlChannel->Attribute("Name");
            if (id)
            {
                channel.id = id;
            }
        }
    }
}

void CZISlide::readMetadata()
{
    // position stream pointer to metadata segment
    m_fileStream.seekg(m_metadataPosition, std::ios_base::beg);
    // read segment header
    SegmentHeader header;
    m_fileStream.read((char*)&header, sizeof(header));
    if (strncmp(header.SID, SID_METADATA, sizeof(SID_METADATA)) != 0)
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

void CZISlide::readFileHeader()
{
    FileHeader fileHeader;
    SegmentHeader header;
    m_fileStream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (strncmp(header.SID, SID_FILES, sizeof(SID_FILES)) != 0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: file %1% is not a CZI file.") % m_filePath).str());
    }
    m_fileStream.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    m_directoryPosition = fileHeader.directoryPosition;
    m_metadataPosition = fileHeader.metadataPosition;
}

void CZISlide::readDirectory()
{
    // position stream pointer to the directory segment
    m_fileStream.seekg(m_directoryPosition, std::ios_base::beg);
    // read segment header
    SegmentHeader header;
    m_fileStream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (strncmp(header.SID, SID_DIRECTORY, sizeof(SID_DIRECTORY)) != 0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: invalid directory segment of file %1%.") % m_filePath).str());
    }
    DirectoryHeader directoryHeader;
    m_fileStream.read(reinterpret_cast<char*>(&directoryHeader), sizeof(directoryHeader));
    std::vector<Blocks> sceneBlocks;
    std::map<uint64_t, int> sceneMap;
    for (unsigned int entry = 0; entry < directoryHeader.entryCount; ++entry)
    {
        Block block;
        DirectoryEntryDV entryHeader;
        m_fileStream.read(reinterpret_cast<char*>(&entryHeader), sizeof(entryHeader));
        block.filePosition = entryHeader.filePosition;
        block.compression = entryHeader.compression;
        block.filePart = entryHeader.filePart;
        block.pixelType = entryHeader.pixelType;
        block.dimensions.reserve(entryHeader.dimensionCount);
        for (int dim = 0; dim < entryHeader.dimensionCount; ++dim)
        {
            DimensionEntryDV dimEntry;
            m_fileStream.read(reinterpret_cast<char*>(&dimEntry), sizeof(dimEntry));
            if (dimEntry.dimension[0] == 'X')
            {
                block.x = dimEntry.start;
                block.width = dimEntry.storedSize;
                block.zoom = dimEntry.storedSize / dimEntry.size;
            }
            else if (dimEntry.dimension[0] == 'Y')
            {
                block.y = dimEntry.start;
                block.height = dimEntry.storedSize;
            }
            else
            {
                if(dimEntry.dimension[0]=='C')
                {
                    block.firstChannel = dimEntry.start;
                    block.lastChannel = dimEntry.start + dimEntry.size - 1;
                }
                Dimension dimension;
                dimension.type = dimEntry.dimension[0];
                dimension.start = dimEntry.start;
                dimension.size = dimEntry.size;
                block.dimensions.push_back(dimension);
                if(dimension.size!=1)
                {
                    throw std::runtime_error(
                        (boost::format("CZIImageDriver: unexpected configuration for dimension '%1%'") % dimension.type).str()
                    );
                }
            }
        }
        block.sceneId = sceneIdFromDims(block.dimensions);
        auto sceneIt = sceneMap.find(block.sceneId);
        int sceneIndex = 0;
        if(sceneIt==sceneMap.end())
        {
            sceneIndex = static_cast<int>(sceneBlocks.size());
            sceneBlocks.emplace_back();
            sceneMap[block.sceneId] = sceneIndex;
        }
        else
        {
            sceneIndex = sceneIt->second;
        }
        sceneBlocks[sceneIndex].push_back(block);
    }
    for(const auto& blocks : sceneBlocks)
    {
        cv::Ptr<CZIScene> scene(new CZIScene);
        scene->init(blocks[0].sceneId, m_filePath, blocks, this);
        m_scenes.push_back(scene);
    }

}

void CZISlide::parseResolutions(XMLNode* root)
{
    const std::vector<std::string> scalingItemsPath = {
        "ImageDocument","Metadata","Scaling", "Items"
    };
    // resolutions
    const XMLElement* xmlItems = getXmlElementByPath(root, scalingItemsPath);
    for (auto child = xmlItems->FirstChildElement(); child != nullptr;
        child = child->NextSiblingElement())
    {
        const char* name = child->Name();
        if (name && strcmp(name, "Distance") == 0)
        {
            const char* id = child->Attribute("Id");
            if (id)
            {
                const XMLElement* valueElement = child->FirstChildElement("Value");
                if (valueElement)
                {
                    const double value = valueElement->DoubleText(0);
                    if (strcmp("X", id) == 0)
                    {
                        m_resX = value;
                    }
                    else if (strcmp("Y", id) == 0)
                    {
                        m_resY = value;
                    }
                    else if (strcmp("Z", id) == 0)
                    {
                        m_resZ = value;
                    }
                }
            }
        }
    }
}

void CZISlide::parseSizes(tinyxml2::XMLNode* root)
{
    const std::vector<std::string> imagePath = { "ImageDocument","Metadata","Information", "Image" };
    const XMLElement* xmlImage = getXmlElementByPath(root, imagePath);
    m_slideXs = xmlChildNodeTextToInt(xmlImage, "SizeX");
    m_slideYs = xmlChildNodeTextToInt(xmlImage, "SizeY");
    m_slideZs = xmlChildNodeTextToInt(xmlImage, "SizeZ");
    m_slideTs = xmlChildNodeTextToInt(xmlImage, "SizeT");
    m_slideRs = xmlChildNodeTextToInt(xmlImage, "SizeR");
    m_slideIs = xmlChildNodeTextToInt(xmlImage, "SizeI");
    m_slideSs = xmlChildNodeTextToInt(xmlImage, "SizeS");
    m_slideHs = xmlChildNodeTextToInt(xmlImage, "SizeH");
    m_slideMs = xmlChildNodeTextToInt(xmlImage, "SizeM");
    m_slideBs = xmlChildNodeTextToInt(xmlImage, "SizeB");
    m_slideVs = xmlChildNodeTextToInt(xmlImage, "SizeV");
}
