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
