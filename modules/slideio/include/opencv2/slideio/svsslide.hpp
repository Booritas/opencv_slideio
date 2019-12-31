// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_svsslide_HPP
#define OPENCV_slideio_svsslide_HPP

#include "opencv2/slideio/scene.hpp"

#include "opencv2/slideio/slide.hpp"
#include "opencv2/slideio/scene.hpp"
#include <tiffio.h>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS SVSSlide : public slideio::Slide
        {
        protected:
            SVSSlide();
        public:
            virtual ~SVSSlide();
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            cv::Ptr<slideio::Scene> getScene(int index) const override;
            static cv::Ptr<SVSSlide> openFile(const std::string& path);
            static void closeFile(TIFF* hfile);
        private:
            std::vector<cv::Ptr<slideio::Scene>> m_Scenes;
            std::string m_filePath;
        };
    }
}
#endif