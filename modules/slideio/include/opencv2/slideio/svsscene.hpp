#pragma once
#include "opencv2/slideio/scene.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS_W SVSScene : public Scene
        {
        public:
            SVSScene(const std::string& filePath, const std::string& name) :
                m_filePath(filePath),
                m_name(name) {
            }
            std::string getScenePath() const override {
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