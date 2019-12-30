#include "test_precomp.hpp"
#include "opencv2/slideio/tifftools.hpp"
#include "testtools.hpp"
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/imgproc.hpp"

namespace opencv_test {

TEST(TiffToolsTest, scanTiffFile)
{
    std::string filePath = TestTools::getTestImagePath("svs","JP2K-33003-1.svs");
    std::vector<slideio::TiffDirectory> dirs;
    slideio::TiffTools::scanFile(filePath, dirs);
    int dirCount = (int)dirs.size();
    ASSERT_EQ(dirCount, 6);
    const slideio::TiffDirectory& dir = dirs[0];
    EXPECT_EQ(dir.width, 15374);
    EXPECT_EQ(dir.height, 17497);
    EXPECT_TRUE(dir.tiled);
    EXPECT_EQ(dir.tileWidth, 256);
    EXPECT_EQ(dir.tileHeight, 256);
    EXPECT_EQ(dir.channels, 3);
    EXPECT_EQ(dir.bitsPerSample, 8);
    EXPECT_EQ(dir.description.size(),530);
    const slideio::TiffDirectory& dir5 = dirs[5];
    EXPECT_EQ(dir5.width, 1280);
    EXPECT_EQ(dir5.height, 421);
    EXPECT_FALSE(dir5.tiled);
    EXPECT_EQ(dir5.tileWidth, 0);
    EXPECT_EQ(dir5.tileHeight, 0);
    EXPECT_EQ(dir5.channels, 3);
    EXPECT_EQ(dir5.bitsPerSample, 8);
    EXPECT_EQ(dir5.description.size(),44);
    EXPECT_TRUE(dir5.interleaved);
    EXPECT_EQ(0, dir5.res.x);
    EXPECT_EQ(0,dir5.res.y);
    EXPECT_EQ(7,dir5.compression);
}

TEST(TiffToolsTest, readStripedDir)
{
    std::string filePathTiff = TestTools::getTestImagePath("svs","CMU-1-Small-Region.svs");
    std::string filePathBmp = TestTools::getTestImagePath("svs", "CMU-1-Small-Region-page-2.bmp");
    TIFF* tiff = TIFFOpen(filePathTiff.c_str(), "r");
    ASSERT_TRUE(tiff!=nullptr);
    int dirIndex = 2;
    ASSERT_TRUE(TIFFSetDirectory(tiff, dirIndex)!=0);
    slideio::TiffDirectory dir;
    dir.dirIndex = dirIndex;
    dir.offset = 0;
    slideio::TiffTools::scanTiffDirTags(tiff, dir);
    dir.dataType = TIFF_BYTE;
    cv::Mat dirRaster;
    slideio::TiffTools::readStripedDir(tiff, dir, dirRaster);
    TIFFClose(tiff);
    cv::Mat image;
    slideio::ImageTools::readGDALImage(filePathBmp, image);
    // compare similarity of rasters from bmp and decoded jp2k file
    cv::Mat score;
    cv::matchTemplate(dirRaster, image, score, cv::TM_CCOEFF_NORMED);
    double minScore(0), maxScore(0);
    cv::minMaxLoc(score, &minScore, &maxScore);
    ASSERT_LT(0.99, minScore);

}

TEST(TiffToolsTest, readTile_jpeg)
{
    const std::string filePath = 
        TestTools::getTestImagePath("svs","CMU-1-Small-Region.svs");
    const std::string tilePath = 
        TestTools::getTestImagePath("svs","CMU-1-Small-Region-page-0-tile_5-5.bmp");
    // read tile from a tiff file
    TIFF* tiff = TIFFOpen(filePath.c_str(), "r");
    ASSERT_TRUE(tiff!=nullptr);
    slideio::TiffDirectory dir;
    dir.dirIndex = 0;
    dir.offset = 0;
    slideio::TiffTools::scanTiffDir(tiff, dir);
    dir.dataType = TIFF_BYTE;
    int tile_sx = (dir.width-1)/dir.tileWidth + 1;
    int tile_sy = (dir.height-1)/dir.tileHeight + 1;
    int tile = 5*tile_sx + 5;
    std::vector<int> channelIndices = {0};
    cv::Mat tileRaster;
    slideio::TiffTools::readTile(tiff, dir, tile, channelIndices, tileRaster);
    TIFFClose(tiff);
    // read extracted tile
    GDALAllRegister();
    GDALDatasetH datasetTile = GDALOpen(tilePath.c_str(), GA_ReadOnly);
    ASSERT_TRUE(datasetTile!=nullptr);
    GDALRasterBandH bandTile = GDALGetRasterBand(datasetTile, 1);
    ASSERT_TRUE(bandTile!=nullptr);
    std::vector<uint8_t> tileBuffer(dir.tileWidth*dir.tileHeight);
    auto read = GDALRasterIO(bandTile, GF_Read, 0,0, dir.tileWidth, dir.tileHeight, tileBuffer.data(), dir.tileWidth, dir.tileHeight, GDT_Byte, 0,0);
    ASSERT_TRUE(read==CE_None);
    GDALClose(datasetTile);
    // compare tiles
    bool equal = std::equal(tileRaster.data, tileRaster.data + tileBuffer.size(), tileBuffer.begin());
    ASSERT_TRUE(equal);
}

TEST(TiffToolsTest, readTile_J2K)
{
    const std::string filePath = 
        TestTools::getTestImagePath("svs","JP2K-33003-1.svs");
    const std::string tilePath = 
        TestTools::getTestImagePath("svs","CMU-1-Small-Region-page-0-tile_5-5.bmp");
    // read tile from a tiff file
    TIFF* tiff = TIFFOpen(filePath.c_str(), "r");
    ASSERT_TRUE(tiff!=nullptr);
    slideio::TiffDirectory dir;
    dir.dirIndex = 0;
    dir.offset = 0;
    slideio::TiffTools::scanTiffDir(tiff, dir);
    dir.dataType = TIFF_BYTE;
    int tile_sx = (dir.width-1)/dir.tileWidth + 1;
    int tile_sy = (dir.height-1)/dir.tileHeight + 1;
    int tile = 5*tile_sx + 5;
    cv::Mat tileRaster;
    std::vector<int> channelIndices;
    slideio::TiffTools::readTile(tiff, dir, tile, channelIndices, tileRaster);
    TIFFClose(tiff);
}

}