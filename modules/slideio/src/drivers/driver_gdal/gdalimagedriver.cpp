// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/imagedriver.hpp"
#include "opencv2/slideio/gdalimagedriver.hpp"
#include "opencv2/slideio/gdalscene.hpp"
#include "opencv2/slideio/gdalslide.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <set>
#pragma warning( push )
#pragma warning(disable:4005)
#include <gdal/gdal.h>
#pragma warning( pop )

using namespace cv;

slideio::GDALImageDriver::GDALImageDriver()
{
	GDALAllRegister();
}

slideio::GDALImageDriver::~GDALImageDriver()
{
}

std::string slideio::GDALImageDriver::getID() const
{
	return std::string("GDAL");
}


cv::Ptr<slideio::Slide> slideio::GDALImageDriver::openFile(const std::string& filePath)
{
	GDALDatasetH ds = GDALScene::openFile(filePath);
	slideio::Slide* slide = new GDALSlide(ds, filePath);
	cv::Ptr<slideio::Slide> ptr(slide);
	return ptr;
}

std::string slideio::GDALImageDriver::getFileSpecs() const
{
	static std::string pattern("*.png;*.jpeg;*.jpg;*.tif;*.tiff;*.bmp;*.gif;*.gtiff;*.gtif;*.ntif;*.jp2");
    return pattern;
}
