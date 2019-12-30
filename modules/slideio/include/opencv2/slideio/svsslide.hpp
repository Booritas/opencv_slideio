#pragma once
#include "opencv2/slideio/slide.hpp"
#include "opencv2/slideio/scene.hpp"
#include <tiffio.h>

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W SVSSlide : public slideio::Slide
        {
        protected:
            SVSSlide();
        public:
            virtual ~SVSSlide();
            int getNumbScenes() const override;
            std::string getFilePath() const override;
            std::shared_ptr<slideio::Scene> getScene(int index) const override;
            static std::shared_ptr<SVSSlide> openFile(const std::string& path);
            static void closeFile(TIFF* hfile);
        private:
            std::vector<std::shared_ptr<slideio::Scene>> m_Scenes;
            std::string m_filePath;
        };
    }
}