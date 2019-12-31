// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_gdalslide_HPP
#define OPENCV_slideio_gdalslide_HPP

#include "opencv2/slideio/slide.hpp"
#pragma warning( push )
#pragma warning(disable:4005)
#include <gdal/gdal.h>
#pragma warning( pop )

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS GDALSlide : public slideio::Slide
        {
        public:
            GDALSlide(GDALDatasetH ds, const std::string& filePath);
            virtual ~GDALSlide();
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            cv::Ptr<slideio::Scene> getScene(int index) const override;
        private:
            cv::Ptr<slideio::Scene> m_scene;
        };

    }
}

#endif
