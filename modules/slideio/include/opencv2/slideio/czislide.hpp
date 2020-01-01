// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_czislide_HPP
#define OPENCV_slideio_czislide_HPP
#include "slide.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS CZISlide : public Slide
        {
        public:
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            cv::Ptr<Scene> getScene(int index) const override;
        };
    }
}

#endif
