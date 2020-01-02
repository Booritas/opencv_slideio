// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/cziimagedriver.hpp"
#include "opencv2/slideio/tools.hpp"
#include "opencv2/slideio/czislide.hpp"

#include <boost/filesystem.hpp>

using namespace cv::slideio;

std::string CZIImageDriver::filePathPattern = "*.czi";

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

cv::Ptr<Slide> CZIImageDriver::openFile(const std::string& filePath)
{
    namespace fs = boost::filesystem;
    if(!fs::exists(filePath)){
        throw std::runtime_error(std::string("CZIImageDriver: File does not exist:") + filePath);
    }
    cv::Ptr<Slide> ptr = new CZISlide(filePath);
    return ptr;
}

std::string CZIImageDriver::getFileSpecs() const
{
    static std::string pattern("*.czi");
    return pattern;
}
