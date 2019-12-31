#pragma once
#include <string>
#include "opencv2/slideio/scene.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W Slide
        {
        public:
            CV_WRAP virtual int getNumbScenes() const = 0;
            CV_WRAP virtual std::string getFilePath() const = 0;
            CV_WRAP virtual cv::Ptr<Scene> getScene(int index) const = 0;
        };
    }
}
