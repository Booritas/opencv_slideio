#pragma once
#include "opencv2/slideio/slide.hpp"
#pragma warning( push )
#pragma warning(disable:4005)
#include <gdal/gdal.h>
#pragma warning( pop )

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W GDALSlide : public slideio::Slide
        {
        public:
            GDALSlide(GDALDatasetH ds, const std::string& filePath);
            virtual ~GDALSlide();
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            std::shared_ptr<slideio::Scene> getScene(int index) const override;
        private:
            std::shared_ptr<slideio::Scene> m_scene;
        };

    }
}
