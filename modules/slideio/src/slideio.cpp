#include "opencv2/slideio/slide.hpp"
#include "opencv2/slideio/imagedrivermanager.hpp"
#include "opencv2/slideio.hpp"
#include <string>
#include <memory>
#include <list>

using namespace cv::slideio;

std::shared_ptr<Slide> openSlide(const std::string& filePath, const std::string& driver)
{
	return ImageDriverManager::openSlide(filePath, driver);
}

std::list<std::string> getDrivers()
{
	std::list<std::string> drivers = ImageDriverManager::getDriverIDs();
	return drivers;
}
