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
        class CV_EXPORTS_W ImageDriverManager
        {
        protected:
            ImageDriverManager();
            ~ImageDriverManager();
        public:
            static std::list<std::string> getDriverIDs();
            static ImageDriver* getDriver(const std::string& driverName);
            static ImageDriver* findDriver(const std::string& filePath);
            static std::shared_ptr<Slide> openSlide(const std::string& cs, const std::string& driver);
        protected:
            static void initialize();
        private:
            static std::map<std::string, std::shared_ptr<ImageDriver>> driverMap;
        };
    }
}
