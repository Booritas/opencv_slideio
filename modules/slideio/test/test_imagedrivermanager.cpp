#include "test_precomp.hpp"
#include "opencv2/slideio/imagedrivermanager.hpp"

namespace opencv_test {

TEST(ImageDriverManagerTest, getDriverIDs)
{
    std::list<std::string> driverIDs = slideio::ImageDriverManager::getDriverIDs();
    EXPECT_FALSE(driverIDs.empty());
}
}