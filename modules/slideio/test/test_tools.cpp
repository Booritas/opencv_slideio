#include "test_precomp.hpp"
#include "testtools.hpp"
#include <boost/dll/runtime_symbol_info.hpp>
#include "opencv2/ts.hpp"

namespace opencv_test {

std::string TestTools::getTestImageDirectory()
{
    boost::filesystem::path testDirPath = cvtest::TS::ptr()->get_data_path();
    testDirPath /= "slideio";
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