// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "test_precomp.hpp"
#include "opencv2/slideio/imagedrivermanager.hpp"
#include "opencv2/slideio/cziimagedriver.hpp"
#include "testtools.hpp"

namespace opencv_test
{

TEST(Slideio_CZIDriver, DriverManager_getDriverIDs)
{
    std::vector<std::string> driverIds = cv::slideio::ImageDriverManager::getDriverIDs();
    auto it = std::find(driverIds.begin(),driverIds.end(), "CZI");
    EXPECT_FALSE(it==driverIds.end());
}
TEST(Slideio_CZIDriver, getID)
{
    slideio::CZIImageDriver driver;
    std::string id = driver.getID();
    EXPECT_EQ(id,"CZI");
}

TEST(Slideio_CZIDriver, canOpenFile)
{
    slideio::CZIImageDriver driver;
    EXPECT_TRUE(driver.canOpenFile("c:\\abbb\\a.czi"));
    EXPECT_FALSE(driver.canOpenFile("c:\\abbb\\a.czi.tmp"));
}

TEST(Slideio_CZIDriver, openFile)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","test.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
}


}
