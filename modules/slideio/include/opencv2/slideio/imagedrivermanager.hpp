// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_imagedrivermanager_HPP
#define OPENCV_slideio_imagedrivermanager_HPP

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
#endif