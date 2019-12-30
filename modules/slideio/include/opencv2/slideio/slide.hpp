#pragma once
#include <string>
#include "opencv2/slideio/scene.hpp"

namespace cv
{
    namespace slideio
    {
        class Slide
        {
        public:
            virtual int getNumbScenes() const = 0;
            virtual std::string getFilePath() const = 0;
            virtual std::shared_ptr<Scene> getScene(int index) const = 0;
        };
    }
}
