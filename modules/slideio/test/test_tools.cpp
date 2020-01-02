// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "test_precomp.hpp"
#include "opencv2/slideio/tools.hpp"

namespace opencv_test {

TEST(Slideio_Tools, matchPattern)
{
    EXPECT_TRUE(cv::slideio::Tools::matchPattern("c:\\abs.ad.czi","*.czi"));
    EXPECT_TRUE(cv::slideio::Tools::matchPattern("c:\\abs.ad.czi","*.tiff;*.czi"));
    EXPECT_FALSE(cv::slideio::Tools::matchPattern("c:\\abs.ad.czi","*.tiff;*.aczi"));
    EXPECT_TRUE(cv::slideio::Tools::matchPattern("c:\\abs.ad.OME.TIFF","*.tiff;*.aczi;*.ome.tiff"));
    EXPECT_TRUE(cv::slideio::Tools::matchPattern("c:\\abs\\SLIDEIO123.OME.TIFF","*.tiff;*.aczi;slideio*.ome.tiff"));
}

}
