// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/svstools.hpp"
#include <string>
#include <regex>

using namespace cv::slideio;

int SVSTools::extractMagnifiation(const std::string& description)
{
	int magn = 0;
	std::regex rgx("\\|AppMag\\s=\\s(\\d+)\\|");
	std::smatch match;
	if(std::regex_search(description, match, rgx)){
		std::string magn_str = match[1];
		magn = std::stoi(magn_str);
	}
	return magn;
}
