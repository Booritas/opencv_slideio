// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/czislide.hpp"
#include "opencv2/slideio/cziscene.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace cv::slideio;

CZISlide::CZISlide(const std::string& filePath) : m_filePath(filePath)
{
    init();
}

int CZISlide::getNumbScenes() const
{
	return 1;
}

std::string CZISlide::getFilePath() const
{
	return m_filePath;
}

cv::Ptr<Scene> CZISlide::getScene(int index) const
{
    if(index<0 || index>=getNumbScenes())
    {
        throw std::runtime_error(
            (boost::format("CZIImageDriver: Invalid scene index %1%") % index).str());
    }

	return m_scene;
}

void CZISlide::init()
{
    CZIScene* scene = new CZIScene;
    m_scene = scene;
    scene->init(m_filePath);
}


