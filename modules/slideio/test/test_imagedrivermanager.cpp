#include "test_precomp.hpp"
#include "opencv2/slideio/imagedrivermanager.hpp"
#include "opencv2/slideio.hpp"

namespace opencv_test {

TEST(Slideio_ImageDriverManager, getDriverIDs)
{
    std::vector<std::string> driverIDs = slideio::ImageDriverManager::getDriverIDs();
    EXPECT_FALSE(driverIDs.empty());
}

TEST(Slideio_ImageDriverManager, getDriversGlobal)
{
    auto drivers = slideio::getDrivers();
    EXPECT_FALSE(drivers.empty());
}

}