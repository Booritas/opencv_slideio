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

std::shared_ptr<slideio::Slide> slideio::SVSImageDriver::openFile(const std::string& filePath)
{
	return SVSSlide::openFile(filePath);
}