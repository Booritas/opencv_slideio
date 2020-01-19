#include "test_precomp.hpp"
#include "testtools.hpp"
#include "opencv2/ts.hpp"

namespace opencv_test {

std::string TestTools::getTestImageDirectory()
{
    std::string testDirPath = cvtest::TS::ptr()->get_data_path();
    testDirPath += "slideio";
    return testDirPath;
}

std::string TestTools::getTestImagePath(const std::string& subfolder, const std::string& image)
{
    std::string imagePath(getTestImageDirectory());
    if(!subfolder.empty())
        imagePath += std::string("/") + subfolder;
    imagePath += std::string("/") +  image;
    return imagePath;
}

}