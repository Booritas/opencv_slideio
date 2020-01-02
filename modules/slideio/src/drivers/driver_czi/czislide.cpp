// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/czislide.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <tinyxml2.h>

using namespace cv::slideio;
static char SID_FILES[] = "ZISRAWFILE";
static char SID_METADATA[] = "ZISRAWMETADATA";

CZISlide::CZISlide(const std::string& filePath) : m_filePath(filePath)
{
    init();
}

int CZISlide::getNumbScenes() const
{
	return 0;
}

std::string CZISlide::getFilePath() const
{
	return "";
}

cv::Ptr<Scene> CZISlide::getScene(int index) const
{
	return nullptr;
}

void CZISlide::init()
{
    // read file header
    m_fileStream.exceptions(std::ios::failbit | std::ios::badbit);
    m_fileStream.open(m_filePath.c_str(), std::ifstream::in|std::ifstream::binary);
    readFileHeader();
    readMetadata();
}

static int xmlChildNodeTextToInt(tinyxml2::XMLElement* xmlParent, const char* childName, int defaultValue=1)
{
    if(xmlParent==nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
    auto xmlChild = xmlParent->FirstChildElement(childName);
    int value = defaultValue;
    if(xmlChild!=nullptr)
        value = xmlChild->IntText(defaultValue);
    return value;
}

void CZISlide::parseMetadataXmL(const char* xmlString, size_t dataSize)
{
    using namespace tinyxml2;
    XMLDocument doc;
    XMLError error = doc.Parse(xmlString, dataSize);
    if(error!= XML_SUCCESS)
    {
        throw std::runtime_error("CZIImageDriver: Error parsing metadata xml");
    }
    auto xmlImageDocument = doc.FirstChildElement("ImageDocument");
    if(xmlImageDocument==nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
    auto xmlMetadata = xmlImageDocument->FirstChildElement("Metadata");
    if(xmlMetadata==nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
    auto xmlInformation = xmlMetadata->FirstChildElement("Information");
    if(xmlInformation==nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
    auto xmlImage = xmlInformation->FirstChildElement("Image");
    if(xmlImage==nullptr)
        throw std::runtime_error("CZIImageDriver: Invalid xml document");
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

void CZISlide::readMetadata()
{
    // position stream pointer to metadata segment
    m_fileStream.seekg(m_fileHeader.metadataPosition, std::ios_base::beg);
    // read segment header
    SegmentHeader header;
    m_fileStream.read((char*)&header, sizeof(header));
    if(strncmp(header.SID,SID_METADATA, sizeof(SID_METADATA))!=0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: file %1% is not a CZI file") % m_filePath).str());
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
    SegmentHeader header;
    m_fileStream.read((char*)&header, sizeof(header));
    if(strncmp(header.SID,SID_FILES, sizeof(SID_FILES))!=0)
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: file %1% is not a CZI file") % m_filePath).str());
    }
    m_fileStream.read((char*)&m_fileHeader, sizeof(m_fileHeader));
}

