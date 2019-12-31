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

bool slideio::SVSImageDriver::canOpenFile(const std::string& filePath) const
{
	namespace fs = boost::filesystem;
	namespace alg = boost::algorithm;
	
	static std::set<std::string> extensions {".svs"};
	 
	fs::path path(filePath);
	std::string ext_str = path.extension().string();
	alg::to_lower(ext_str);
	const bool found = extensions.find(ext_str)!=extensions.end();
	return found;
}

cv::Ptr<slideio::Slide> slideio::SVSImageDriver::openFile(const std::string& filePath)
{
	return SVSSlide::openFile(filePath);
}
