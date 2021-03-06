// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/svsimagedriver.hpp"
#include "opencv2/slideio/svsslide.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <set>

using namespace cv;

slideio::SVSImageDriver::SVSImageDriver()
{
}

slideio::SVSImageDriver::~SVSImageDriver()
{
}

std::string slideio::SVSImageDriver::getID() const
{
	return std::string("SVS");
}

cv::Ptr<slideio::Slide> slideio::SVSImageDriver::openFile(const std::string& filePath)
{
	return SVSSlide::openFile(filePath);
}

std::string slideio::SVSImageDriver::getFileSpecs() const
{
	static std::string pattern("*.svs");
    return pattern;
}
