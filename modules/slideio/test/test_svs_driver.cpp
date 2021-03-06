#include "test_precomp.hpp"
#include "opencv2/slideio/svsimagedriver.hpp"
#include "opencv2/slideio/svstiledscene.hpp"
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "testtools.hpp"
#include <stdint.h>
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

namespace opencv_test {

TEST(Slideio_SVSImageDriver, driverID)
{
    slideio::SVSImageDriver driver;
    EXPECT_EQ(driver.getID(), "SVS");
}

TEST(Slideio_SVSImageDriver, canOpenFile)
{
    slideio::SVSImageDriver driver;
    EXPECT_TRUE(driver.canOpenFile("abc.svs"));
    EXPECT_FALSE(driver.canOpenFile("abc.tif"));
}

TEST(Slideio_SVSImageDriver, openFile_BrightField)
{
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs","CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide!=nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes==4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(0);
    ASSERT_TRUE(scene!=nullptr);
    EXPECT_EQ(slide->getFilePath(),path);
    EXPECT_EQ(scene->getFilePath(),path);
    int channels = scene->getNumChannels();
    EXPECT_EQ(channels, 3);
    cv::Rect sceneRect = scene->getRect();
    EXPECT_EQ(sceneRect.width, 2220);
    EXPECT_EQ(sceneRect.height, 2967);
    EXPECT_EQ(scene->getChannelDataType(0), slideio::DataType::DT_Byte);
    EXPECT_EQ(scene->getChannelDataType(1), slideio::DataType::DT_Byte);
    EXPECT_EQ(scene->getChannelDataType(2), slideio::DataType::DT_Byte);
    slideio::Resolution res = scene->getResolution();
    EXPECT_EQ(res.x, 0.);
    EXPECT_EQ(res.y, 0.);
    double magn = scene->getMagnification();
    EXPECT_EQ(20., magn);
}

TEST(Slideio_SVSImageDriver, read_Thumbnail_WholeImage)
{
    // read image by svs driver
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs","CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide!=nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes==4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(1);
    ASSERT_TRUE(scene!=nullptr);
    cv::Rect sceneRect = scene->getRect();
    cv::Mat imageRaster;
    scene->readBlock(sceneRect, imageRaster);
    
    // read extracted page by GDAL library
    std::string pathPageFile = TestTools::getTestImagePath("svs","CMU-1-Small-Region-page-1.tif");
    cv::Mat pageRaster;
    slideio::ImageTools::readGDALImage(pathPageFile, pageRaster);

    cv::Mat score;
    cv::matchTemplate(imageRaster, pageRaster, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    ASSERT_LT(0.99, minScore);
}

TEST(Slideio_SVSImageDriver, read_Thumbnail_Block)
{
    // read image by svs driver
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs","CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide!=nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes==4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(1);
    ASSERT_TRUE(scene!=nullptr);
    cv::Rect sceneRect = scene->getRect();
    int block_sx = sceneRect.width/4;
    int block_sy = sceneRect.height/3;
    int block_x = sceneRect.width/6;
    int block_y = sceneRect.height/5;
    
    cv::Rect blockRect = {block_x,block_y,block_sx, block_sy};
    cv::Mat blockRaster;
    scene->readBlock(blockRect, blockRaster);
    ASSERT_EQ(blockRaster.cols, block_sx);
    ASSERT_EQ(blockRaster.rows, block_sy);

    // read extracted page by GDAL library
    std::string pathPageFile = TestTools::getTestImagePath("svs", "CMU-1-Small-Region-page-1.tif");
    cv::Mat pageRaster;
    slideio::ImageTools::readGDALImage(pathPageFile, pageRaster);
    cv::Mat pageBlockRaster = pageRaster(blockRect);

    cv::Mat score;
    cv::matchTemplate(blockRaster, pageBlockRaster, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    ASSERT_LT(0.99, minScore);
}

TEST(Slideio_SVSImageDriver, read_Thumbnail_BlockWithScale)
{
    // read image by svs driver
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs", "CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide != nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes == 4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(1);
    ASSERT_TRUE(scene != nullptr);
    cv::Rect sceneRect = scene->getRect();
    int block_sx = sceneRect.width/3;
    int block_sy = sceneRect.height/2;
    int block_x = sceneRect.width/6;
    int block_y = sceneRect.height/5;
    double scale = 0.8;

    cv::Rect blockRect = { block_x,block_y,block_sx, block_sy };
    cv::Size blockSize = { (int)(sceneRect.width * scale), (int)(sceneRect.height * scale) };

    cv::Mat blockRaster;
    scene->readResampledBlock(blockRect, blockSize, blockRaster);
    ASSERT_EQ(blockRaster.cols, blockSize.width);
    ASSERT_EQ(blockRaster.rows, blockSize.height);

    // read extracted page by GDAL library
    std::string pathPageFile = TestTools::getTestImagePath("svs", "CMU-1-Small-Region-page-1.tif");
    cv::Mat pageRaster;
    slideio::ImageTools::readGDALImage(pathPageFile, pageRaster);
    cv::Mat pageBlockRaster = pageRaster(blockRect);
    cv::Mat pageResizedRaster;
    cv::resize(pageBlockRaster, pageResizedRaster, blockSize);

    cv::Mat score;
    cv::matchTemplate(blockRaster, pageResizedRaster, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    ASSERT_LT(0.99, minScore);
}

TEST(Slideio_SVSImageDriver, findZoomDirectory)
{
    std::vector<slideio::TiffDirectory> dirs;
    dirs.resize(10);
    int baseWidth = 38528;
    int baseHeight = 77056;
    int scale = 1;
    int index = 0;
    for(auto& dir: dirs)
    {
        dir.width = baseWidth / scale;
        dir.height = baseHeight / scale;
        dir.dirIndex = index;
        scale *= 2;
        index++;
    }
    slideio::SVSTiledScene scene("path", "name", dirs, nullptr);
    auto& lastDir = dirs[dirs.size()-1];
    const cv::Rect sceneRect = scene.getRect();
    double lastZoom = static_cast<double>(lastDir.width) / static_cast<double>(sceneRect.width);

    EXPECT_EQ(scene.findZoomDirectory(2.).dirIndex, 0);
    EXPECT_EQ(scene.findZoomDirectory(lastZoom).dirIndex, 9);
    EXPECT_EQ(scene.findZoomDirectory(lastZoom*2).dirIndex, 8);
    EXPECT_EQ(scene.findZoomDirectory(lastZoom/2).dirIndex, 9);
    EXPECT_EQ(scene.findZoomDirectory(0.5).dirIndex, 1);
    EXPECT_EQ(scene.findZoomDirectory(0.501).dirIndex, 1);
    EXPECT_EQ(scene.findZoomDirectory(0.499).dirIndex, 1);
    EXPECT_EQ(scene.findZoomDirectory(0.25).dirIndex, 2);
    EXPECT_EQ(scene.findZoomDirectory(0.125).dirIndex, 3);
    EXPECT_EQ(scene.findZoomDirectory(0.55).dirIndex, 0);
    EXPECT_EQ(scene.findZoomDirectory(0.45).dirIndex, 1);
    EXPECT_EQ(scene.findZoomDirectory(0.1).dirIndex, 3);
}

TEST(Slideio_SVSImageDriver, readBlock_WholeImage)
{
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs", "CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide != nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes == 4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(0);
    ASSERT_TRUE(scene != nullptr);
    const cv::Rect sceneRect = scene->getRect();
    ASSERT_EQ(sceneRect.width, 2220);
    ASSERT_EQ(sceneRect.height, 2967);
    cv::Mat sceneRaster;
    scene->readBlock(sceneRect, sceneRaster);

    //namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    //imshow( "Display window", sceneRaster );                   // Show our image inside it.
    //waitKey(0);

    // read extracted page by GDAL library
    std::string pathPageFile = TestTools::getTestImagePath("svs", "CMU-1-Small-Region-page-0.tif");
    cv::Mat pageRaster;
    slideio::ImageTools::readGDALImage(pathPageFile, pageRaster);
    cv::Mat pageBlockRaster = pageRaster(sceneRect);
    cv::Mat score;
    cv::matchTemplate(sceneRaster, pageBlockRaster, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    ASSERT_LT(0.99, minScore);
}

TEST(Slideio_SVSImageDriver, readBlock_Part)
{
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs", "CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide != nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes == 4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(0);
    ASSERT_TRUE(scene != nullptr);
    const cv::Rect sceneRect = scene->getRect();
    ASSERT_EQ(sceneRect.width, 2220);
    ASSERT_EQ(sceneRect.height, 2967);
    cv::Mat blockRaster;
    cv::Rect blockRect = {sceneRect.width/2, sceneRect.height/2, 300, 300};
    scene->readBlock(blockRect, blockRaster);

    // read extracted page by GDAL library
    std::string pathPageFile = TestTools::getTestImagePath("svs", "CMU-1-Small-Region-page-0.tif");
    cv::Mat pageRaster;
    slideio::ImageTools::readGDALImage(pathPageFile, pageRaster);
    cv::Mat pageBlockRaster = pageRaster(blockRect);
    cv::Mat score;
    cv::matchTemplate(blockRaster, pageBlockRaster, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    ASSERT_LT(0.99, minScore);
}

TEST(Slideio_SVSImageDriver, readBlock_PartScale)
{
    slideio::SVSImageDriver driver;
    std::string path = TestTools::getTestImagePath("svs", "CMU-1-Small-Region.svs");
    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
    ASSERT_TRUE(slide != nullptr);
    int numbScenes = slide->getNumbScenes();
    ASSERT_TRUE(numbScenes == 4);
    std::shared_ptr<slideio::Scene> scene = slide->getScene(0);
    ASSERT_TRUE(scene != nullptr);
    const cv::Rect sceneRect = scene->getRect();
    ASSERT_EQ(sceneRect.width, 2220);
    ASSERT_EQ(sceneRect.height, 2967);
    cv::Mat blockRaster;
    cv::Rect blockRect = { sceneRect.width / 2, sceneRect.height / 2, 300, 300 };
    cv::Size blockSize = blockRect.size();
    blockSize /= 2;
    scene->readResampledBlock(blockRect, blockSize, blockRaster);

    // read extracted page by GDAL library
    std::string pathPageFile = TestTools::getTestImagePath("svs", "CMU-1-Small-Region-page-0.tif");
    cv::Mat pageRaster;
    slideio::ImageTools::readGDALImage(pathPageFile, pageRaster);
    cv::Mat pageBlockRaster = pageRaster(blockRect);
    cv::Mat scaledRaster;
    cv::resize(pageBlockRaster, scaledRaster, blockSize);
    cv::Mat score;
    cv::matchTemplate(blockRaster, scaledRaster, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    EXPECT_LT(0.98, minScore);
    //{
    //    namedWindow( "svs", WINDOW_AUTOSIZE );
    //    imshow( "svs", blockRaster );
    //    namedWindow( "gdal", WINDOW_AUTOSIZE );
    //    imshow( "gdal", scaledRaster ); 
    //    cv::Mat difmat;
    //    difmat = scaledRaster - blockRaster;
    //    namedWindow( "diff", WINDOW_AUTOSIZE );
    //    imshow( "diff", difmat ); 
    //    waitKey(0);
    //}
}
//TEST(SVSImageDriver, composeRect2)
//{
//    slideio::SVSImageDriver driver;
//    std::string path = TestTools::getTestImagePath("svs", "JP2K-33003-1.svs");
//    std::shared_ptr<slideio::Slide> slide = driver.openFile(path);
//    ASSERT_TRUE(slide != nullptr);
//    int numbScenes = slide->getNumbScenes();
//    ASSERT_TRUE(numbScenes == 4);
//    std::shared_ptr<slideio::Scene> scene = slide->getScene(0);
//    ASSERT_TRUE(scene != nullptr);
//    const cv::Rect sceneRect = scene->getRect();
//    cv::Mat blockRaster;
//    cv::Size blockSize = { sceneRect.width /3, sceneRect.height / 3};
//    cv::Mat image;
//    scene->readResampledBlock(sceneRect, blockSize, image);
//
//    cv::imwrite(R"(c:\Temp\a.bmp)", image);
//}

}