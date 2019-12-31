#include "opencv2/slideio/slide.hpp"
#include "opencv2/slideio/imagedrivermanager.hpp"
#include "opencv2/slideio.hpp"
#include <string>
#include <memory>
#include <list>

using namespace cv::slideio;

cv::Ptr<Slide> cv::slideio::openSlide(const cv::String& filePath, const cv::String& driver)
{
    return ImageDriverManager::openSlide(filePath, driver);
}

std::vector<cv::String> cv::slideio::getDrivers()
{
    return ImageDriverManager::getDriverIDs();
}
