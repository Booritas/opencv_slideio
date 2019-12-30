#include "opencv2/slideio/imagedriver.hpp"
#include "opencv2/slideio/gdalimagedriver.hpp"
#include "opencv2/slideio/gdalscene.hpp"
#include "opencv2/slideio/gdalslide.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <set>
#pragma warning( push )
#pragma warning(disable:4005)
#include <gdal/gdal.h>
#pragma warning( pop )

using namespace cv;

slideio::GDALImageDriver::GDALImageDriver()
{
	GDALAllRegister();
}

slideio::GDALImageDriver::~GDALImageDriver()
{
}

std::string slideio::GDALImageDriver::getID() const
{
	return std::string("GDAL");
}

bool slideio::GDALImageDriver::canOpenFile(const std::string& filePath) const
{
	namespace fs = boost::filesystem;
	namespace alg = boost::algorithm;
	
	static std::set<std::string> extensions {".png", ".jpeg", ".jpg",
		".tif", ".tiff", ".bmp", ".gif", ".gtiff", ".gtif", ".ntif",
		".jp2"};
	 
	fs::path path(filePath);
	std::string ext_str = path.extension().string();
	alg::to_lower(ext_str);
	const bool found = extensions.find(ext_str)!=extensions.end();
	return found;
}

std::shared_ptr<slideio::Slide> slideio::GDALImageDriver::openFile(const std::string& filePath)
{
	GDALDatasetH ds = GDALScene::openFile(filePath);
	slideio::Slide* slide = new GDALSlide(ds, filePath);
	std::shared_ptr<slideio::Slide> ptr(slide);
	return ptr;
}