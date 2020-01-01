// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/cziimagedriver.hpp"
#include <codecvt>

using namespace cv::slideio;

CZIImageDriver::CZIImageDriver()
{
}

CZIImageDriver::~CZIImageDriver()
{
}

std::string CZIImageDriver::getID() const
{
    return "CZI";
}

bool CZIImageDriver::canOpenFile(const std::string& filePath) const
{
    return false;
}

cv::Ptr<Slide> CZIImageDriver::openFile(const std::string& filePath)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring path = converter.from_bytes(filePath);
    auto stream = libCZI::CreateStreamFromFile(path.c_str());
    return nullptr;
}
