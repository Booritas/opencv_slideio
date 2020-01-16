// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_svsscene_HPP
#define OPENCV_slideio_svsscene_HPP

#include "opencv2/slideio/scene.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS SVSScene : public Scene
        {
        public:
            SVSScene(const std::string& filePath, const std::string& name) :
                m_filePath(filePath),
                m_name(name) {
            }
            std::string getFilePath() const override {
                return m_filePath;
            }
            std::string getName() const override {
                return m_name;
            }
        protected:
            std::string m_filePath;
            std::string m_name;
        };
    }
}
#endif
