#include "test_precomp.hpp"
#include "testtools.hpp"
#include <boost/dll/runtime_symbol_info.hpp>

namespace opencv_test {

std::string TestTools::getTestImageDirectory()
{
	boost::filesystem::path modulePath = boost::dll::program_location();
	boost::filesystem::path testDirPath;
	for(const auto& item : modulePath)
	{
		if(item.string().compare("bin")==0){
			testDirPath /= "TestImages";
			break;
		}
		else{
			testDirPath /= item;
		}
	}
	return testDirPath.string();
}

std::string TestTools::getTestImagePath(const std::string& subfolder, const std::string& image)
{
	boost::filesystem::path imagePath(getTestImageDirectory());
	if(!subfolder.empty())
		imagePath /= subfolder;
	imagePath /= image;
	return imagePath.string();
}

}