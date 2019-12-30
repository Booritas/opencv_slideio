#include "opencv2/slideio/imagedrivermanager.hpp"
#include "opencv2/slideio/gdalimagedriver.hpp"

using namespace cv::slideio;
std::map<std::string, std::shared_ptr<ImageDriver>> ImageDriverManager::driverMap;


ImageDriverManager::ImageDriverManager()
{
}

ImageDriverManager::~ImageDriverManager()
{
}

std::list<std::string> ImageDriverManager::getDriverIDs()
{
	initialize();
	std::list<std::string> ids;
	for(const auto drv : driverMap){
		ids.push_back(drv.first);
	}
	return ids;
}

void ImageDriverManager::initialize()
{
	if(driverMap.empty())
	{
		GDALImageDriver* driver = new GDALImageDriver;
		std::shared_ptr<ImageDriver> gdal(driver);
		driverMap[gdal->getID()] = gdal;
	}
}

std::shared_ptr<Slide> ImageDriverManager::openSlide(const std::string& filePath, const std::string& driverName)
{
	auto it = driverMap.find(driverName);
	if(it==driverMap.end())
		throw std::runtime_error("ImageDriverManager: Unknown driver " + driverName);
	std::shared_ptr<slideio::ImageDriver> driver = it->second;
	return driver->openFile(filePath);
}
