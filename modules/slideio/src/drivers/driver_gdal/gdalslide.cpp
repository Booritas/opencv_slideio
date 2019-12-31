#include "opencv2/slideio/gdalslide.hpp"
#include "opencv2/slideio/gdalscene.hpp"
#include <boost/filesystem.hpp>

using namespace cv;

slideio::GDALSlide::GDALSlide(GDALDatasetH ds, const std::string& filePath)
{
	m_scene.reset(new slideio::GDALScene(ds, filePath));
}

slideio::GDALSlide::~GDALSlide()
{
}

int slideio::GDALSlide::getNumbScenes() const
{
	return (m_scene==nullptr)?0:1;
}

std::string slideio::GDALSlide::getFilePath() const
{
	if(m_scene!=nullptr)
		return m_scene->getScenePath();
	
	std::string empty_path;
	return empty_path;
}

cv::Ptr<slideio::Scene> slideio::GDALSlide::getScene(int index) const
{
	if(index>=getNumbScenes())
		throw std::runtime_error("GDAL driver: invalide m_scene index");
	return m_scene;
}

