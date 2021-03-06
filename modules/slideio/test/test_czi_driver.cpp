﻿// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "test_precomp.hpp"
#include "opencv2/slideio/imagedrivermanager.hpp"
#include "opencv2/slideio/cziimagedriver.hpp"
#include "testtools.hpp"
#include "opencv2/slideio/czislide.hpp"

namespace opencv_test
{

TEST(Slideio_CZIImageDriver, DriverManager_getDriverIDs)
{
    std::vector<std::string> driverIds = cv::slideio::ImageDriverManager::getDriverIDs();
    auto it = std::find(driverIds.begin(),driverIds.end(), "CZI");
    EXPECT_FALSE(it==driverIds.end());
}
TEST(Slideio_CZIImageDriver, getID)
{
    slideio::CZIImageDriver driver;
    std::string id = driver.getID();
    EXPECT_EQ(id,"CZI");
}

TEST(Slideio_CZIImageDriver, canOpenFile)
{
    slideio::CZIImageDriver driver;
    EXPECT_TRUE(driver.canOpenFile("c:\\abbb\\a.czi"));
    EXPECT_FALSE(driver.canOpenFile("c:\\abbb\\a.czi.tmp"));
}

TEST(Slideio_CZIImageDriver, openFile)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","pJP31mCherry.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
    int numScenes = slide->getNumbScenes();
    ASSERT_EQ(numScenes, 1);
    auto scene = slide->getScene(0);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    EXPECT_EQ(sceneRect.x, 0);
    EXPECT_EQ(sceneRect.y, 0);
    EXPECT_EQ(sceneRect.width, 512);
    EXPECT_EQ(sceneRect.height, 512);
    int numChannels = scene->getNumChannels();
    EXPECT_EQ(numChannels, 3);
    for(int channel=0; channel<numChannels; ++channel)
    {
        EXPECT_EQ(scene->getChannelDataType(channel), cv::slideio::DataType::DT_Byte);
    }
    EXPECT_EQ(scene->getMagnification(), 100.);
    cv::slideio::Resolution res = scene->getResolution();
    const double fileRes = 9.76783e-8;
    EXPECT_LT((100 * std::abs(res.x - fileRes) / fileRes), 1);
    EXPECT_LT((100 * std::abs(res.y - fileRes) / fileRes), 1);
}

TEST(Slideio_CZIImageDriver, openFile2)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi", "08_18_2018_enc_1001_633.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide != nullptr);
    int numScenes = slide->getNumbScenes();
    ASSERT_EQ(numScenes, 1);
    auto scene = slide->getScene(0);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    EXPECT_EQ(sceneRect.x, 0);
    EXPECT_EQ(sceneRect.y, 0);
    EXPECT_EQ(sceneRect.width, 1000);
    EXPECT_EQ(sceneRect.height, 1000);
    int numChannels = scene->getNumChannels();
    EXPECT_EQ(numChannels, 6);
    for (int channel = 0; channel < numChannels; ++channel)
    {
        EXPECT_EQ(scene->getChannelDataType(channel), cv::slideio::DataType::DT_UInt16);
    }
    EXPECT_EQ(scene->getMagnification(), 63.);
    cv::slideio::Resolution res = scene->getResolution();
    const double fileRes = 6.7475572821478794e-008;
    EXPECT_LT((100 * std::abs(res.x - fileRes) / fileRes), 1);
    EXPECT_LT((100 * std::abs(res.y - fileRes) / fileRes), 1);
    EXPECT_STREQ(scene->getChannelName(2).c_str(), "664");
    std::string sceneName = scene->getName();
    int a = 0;
}

TEST(Slideio_CZIImageDriver, readBlock)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","pJP31mCherry.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
    int numScenes = slide->getNumbScenes();
    ASSERT_EQ(numScenes, 1);
    auto scene = slide->getScene(0);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    cv::Mat raster;
    std::vector<int> channelIndices = {0,1,2};
    scene->readBlockChannels(sceneRect,channelIndices,raster);
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", raster);
    waitKey(0);
}

TEST(Slideio_CZIImageDriver, readBlock2)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","03_15_2019_DSGN0549_C_fov_9_633.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
    int numScenes = slide->getNumbScenes();
    auto scene = slide->getScene(0);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    cv::Mat raster;
    std::vector<int> channelIndices = {0,1,2};
    cv::Size size = sceneRect.size();
    size.width/=3;
    size.height/=3;
    scene->readResampledBlockChannels(sceneRect, size, channelIndices, raster);
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", raster);
    waitKey(0);
}

    TEST(Slideio_CZIImageDriver, openFile3)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi", "test3.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide != nullptr);
    int numScenes = slide->getNumbScenes();
    ASSERT_EQ(numScenes, 3);
    for(int sceneIndex=0; sceneIndex<numScenes; ++sceneIndex)
    {
        auto scene = slide->getScene(sceneIndex);
        ASSERT_FALSE(scene == nullptr);
        auto sceneRect = scene->getRect();
        int numChannels = scene->getNumChannels();
        auto sceneName = scene->getName();
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto dt = scene->getChannelDataType(channel);
            auto name = scene->getChannelName(channel);
        }
    }
}

TEST(Slideio_CZIImageDriver, sceneId)
{
    {
        int values[] = { 1, 2, 3, 4, 5, 6 };
        int values2[6] = { 0 };
        uint64_t sceneId = slideio::CZIScene::sceneIdFromDims(values[0], values[1], values[2], values[3], values[4], values[5]);
        slideio::CZIScene::dimsFromSceneId(sceneId, values2[0], values2[1], values2[2], values2[3], values2[4], values2[5]);
        for (int val = 0; val < 6; val++)
        {
            EXPECT_EQ(values[val], values2[val]);
        }
    }
    {
        int values[] = { 1, 0, 0, 0, 0, 0 };
        int values2[6] = { 0 };
        uint64_t sceneId = slideio::CZIScene::sceneIdFromDims(values[0], values[1], values[2], values[3], values[4], values[5]);
        slideio::CZIScene::dimsFromSceneId(sceneId, values2[0], values2[1], values2[2], values2[3], values2[4], values2[5]);
        for (int val = 0; val < 6; val++)
        {
            EXPECT_EQ(values[val], values2[val]);
        }
    }
    {
        int values[] = { 0, 0, 4, 0, 0, 0 };
        int values2[6] = { 0 };
        uint64_t sceneId = slideio::CZIScene::sceneIdFromDims(values[0], values[1], values[2], values[3], values[4], values[5]);
        slideio::CZIScene::dimsFromSceneId(sceneId, values2[0], values2[1], values2[2], values2[3], values2[4], values2[5]);
        for (int val = 0; val < 6; val++)
        {
            EXPECT_EQ(values[val], values2[val]);
        }
    }
    {
        int values[] = { 0, 0, 0, 0, 10, 0 };
        int values2[6] = { 0 };
        uint64_t sceneId = slideio::CZIScene::sceneIdFromDims(values[0], values[1], values[2], values[3], values[4], values[5]);
        slideio::CZIScene::dimsFromSceneId(sceneId, values2[0], values2[1], values2[2], values2[3], values2[4], values2[5]);
        for (int val = 0; val < 6; val++)
        {
            EXPECT_EQ(values[val], values2[val]);
        }
    }
}
TEST(Slideio_CZIImageDriver, sceneIdsFromDims)
{
    {
        std::vector<slideio::Dimension> dims = {
            {'V',1,1},
            {'H',2,1},
            {'I',3,1},
            {'R',4,1},
            {'B',5,1},
            {'S',6,1},
        };
        std::vector<uint64_t> ids;
        slideio::CZIScene::sceneIdsFromDims(dims, ids);
        auto sceneId = slideio::CZIScene::sceneIdFromDims(6,3,1,2,4,5);
        EXPECT_EQ(ids.size(),1);
        EXPECT_EQ(ids[0], sceneId);
    }
    {
        std::vector<slideio::Dimension> dims = {
            {'V',1,1},
            {'H',2,1},
            {'I',3,1},
            {'R',4,1},
            {'B',5,1},
            {'S',6,2},
        };
        std::vector<uint64_t> ids;
        slideio::CZIScene::sceneIdsFromDims(dims, ids);
        auto sceneId1 = slideio::CZIScene::sceneIdFromDims(6,3,1,2,4,5);
        auto sceneId2 = slideio::CZIScene::sceneIdFromDims(7,3,1,2,4,5);
        EXPECT_EQ(ids.size(),2);
        EXPECT_EQ(ids[0], sceneId1);
        EXPECT_EQ(ids[1], sceneId2);
    }
    {
        std::vector<slideio::Dimension> dims = {
            {'V',1,2},
            {'S',6,2},
        };
        std::vector<uint64_t> ids;
        slideio::CZIScene::sceneIdsFromDims(dims, ids);
        auto sceneId1 = slideio::CZIScene::sceneIdFromDims(6,0,1,0,0,0);
        auto sceneId2 = slideio::CZIScene::sceneIdFromDims(6,0,2,0,0,0);
        auto sceneId3 = slideio::CZIScene::sceneIdFromDims(7,0,1,0,0,0);
        auto sceneId4 = slideio::CZIScene::sceneIdFromDims(7,0,2,0,0,0);
        EXPECT_EQ(ids.size(),4);
        EXPECT_TRUE(std::find(ids.begin(), ids.end(),sceneId1)!=ids.end());
        EXPECT_TRUE(std::find(ids.begin(), ids.end(),sceneId2)!=ids.end());
        EXPECT_TRUE(std::find(ids.begin(), ids.end(),sceneId3)!=ids.end());
        EXPECT_TRUE(std::find(ids.begin(), ids.end(),sceneId4)!=ids.end());
    }
    {
        std::vector<slideio::Dimension> dims = {
            {'V',1,2},
            {'H',2,2},
            {'I',3,2},
            {'R',4,2},
            {'B',5,2},
            {'S',6,2},
        };
        std::vector<uint64_t> ids;
        slideio::CZIScene::sceneIdsFromDims(dims, ids);
        EXPECT_EQ(ids.size(),64);
    }
}

TEST(Slideio_CZIImageDriver, readBlock3)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","test3.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
    int numScenes = slide->getNumbScenes();
    auto scene = slide->getScene(1);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    cv::Mat raster;
    std::vector<int> channelIndices;
    cv::Size size = sceneRect.size();
    scene->readResampledBlockChannels(sceneRect, size, channelIndices, raster);
    double dmax, dmin;
    cv::minMaxLoc(raster, &dmin, &dmax);
    cv::Mat dst;
    raster.convertTo(dst,CV_8U,255./dmax, 0);
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", dst);
    waitKey(0);
}

TEST(Slideio_CZIImageDriver, readBlock4)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","test.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
    int numScenes = slide->getNumbScenes();
    auto scene = slide->getScene(0);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    cv::Mat raster;
    std::vector<int> channelIndices = {0};
    cv::Size size = sceneRect.size();
    size.width/=4;
    size.height/=4;
    scene->readResampledBlockChannels(sceneRect, size, channelIndices, raster);
    double dmax, dmin;
    cv::minMaxLoc(raster, &dmin, &dmax);
    cv::Mat dst;
    raster.convertTo(dst,CV_8U,255./dmax, 0);
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", dst);
    waitKey(0);
}

TEST(Slideio_CZIImageDriver, readBlock5)
{
    slideio::CZIImageDriver driver;
    std::string filePath = TestTools::getTestImagePath("czi","test2.czi");
    cv::Ptr<slideio::Slide> slide = driver.openFile(filePath);
    ASSERT_TRUE(slide!=nullptr);
    int numScenes = slide->getNumbScenes();
    auto scene = slide->getScene(0);
    ASSERT_FALSE(scene == nullptr);
    auto sceneRect = scene->getRect();
    cv::Mat raster;
    std::vector<int> channelIndices = {0};
    cv::Size size = sceneRect.size();
    size.width/=4;
    size.height/=4;
    scene->readResampledBlockChannels(sceneRect, size, channelIndices, raster);
    double dmax, dmin;
    cv::minMaxLoc(raster, &dmin, &dmax);
    cv::Mat dst;
    raster.convertTo(dst,CV_8U,255./dmax, 0);
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", dst);
    waitKey(0);
}

}
