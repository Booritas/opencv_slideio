// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/czislide.hpp"

using namespace cv::slideio;

int CZISlide::getNumbScenes() const
{
	return 0;
}

std::string CZISlide::getFilePath() const
{
	return "";
}

cv::Ptr<Scene> CZISlide::getScene(int index) const
{
	return nullptr;
}
