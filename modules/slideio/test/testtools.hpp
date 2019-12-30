#pragma once
#include <string>

namespace opencv_test 
{
	class TestTools
	{
	public:
		static std::string getTestImageDirectory();
		static std::string getTestImagePath(const std::string& subfolder, const std::string& image);
	};
}

