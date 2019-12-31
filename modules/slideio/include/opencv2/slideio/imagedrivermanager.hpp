#pragma once
#include "opencv2/slideio/slide.hpp"
#include <list>
#include <map>
#include <memory>

namespace cv
{
    namespace slideio
    {
        class ImageDriver;
        class CV_EXPORTS ImageDriverManager
        {
        protected:
            ImageDriverManager();
            ~ImageDriverManager();
        public:
            static std::vector<std::string> getDriverIDs();
            static ImageDriver* getDriver(const std::string& driverName);
            static ImageDriver* findDriver(const std::string& filePath);
            static cv::Ptr<Slide> openSlide(const std::string& cs, const std::string& driver);
        protected:
            static void initialize();
        private:
            static std::map<std::string, cv::Ptr<ImageDriver>> driverMap;
        };
    }
}
