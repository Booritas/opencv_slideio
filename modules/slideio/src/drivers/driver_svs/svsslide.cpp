// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/svsslide.hpp"
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/slideio/svssmallscene.hpp"
#include "opencv2/slideio/svstiledscene.hpp"

#include <boost/filesystem.hpp>


using namespace cv::slideio;

SVSSlide::SVSSlide()
{
}

SVSSlide::~SVSSlide()
{
}

int SVSSlide::getNumbScenes() const
{
    return (int)m_Scenes.size();
}

std::string SVSSlide::getFilePath() const
{
    return m_filePath;
}

cv::Ptr<Scene> SVSSlide::getScene(int index) const
{
    if(index>=getNumbScenes())
        throw std::runtime_error("SVS driver: invalide m_scene index");
    return m_Scenes[index];
}

cv::Ptr<SVSSlide> SVSSlide::openFile(const std::string& filePath)
{
    namespace fs = boost::filesystem;
    cv::Ptr<SVSSlide> slide;
    if(!fs::exists(filePath)){
        throw std::runtime_error(std::string("SVSImageDriver: File does not exist:") + filePath);
    }
    std::vector<TiffDirectory> directories;
    TIFF* tiff(nullptr);
    tiff = TIFFOpen(filePath.c_str(), "r");
    if(!tiff)
        return slide;
    
    TiffTools::scanFile(tiff, directories);
    std::vector<int> image;
    int thumbnail(-1), macro(-1), label(-1);
    image.push_back(0); //base image
    int nextDir = 1;
    if(!directories[nextDir].tiled){
        thumbnail = nextDir;
        nextDir++;
    }
    for(int dir=nextDir; dir<directories.size(); dir++)
    {
        auto directory = directories[dir];
        if(!directory.tiled)
            break;
        image.push_back(dir);
        nextDir++;
    }
    for(;nextDir<directories.size(); nextDir++)
    {
        auto directory = directories[nextDir];
        if(directory.description.find("label")!=std::string::npos)
            label = nextDir;
        else if(directory.description.find("macro")!=std::string::npos)
            macro = nextDir;
    }
    std::vector<cv::Ptr<Scene>> scenes;
    
    if(image.size()>0){
        std::vector<TiffDirectory> image_dirs;
        for(const auto index: image){
            image_dirs.push_back(directories[index]);
        }
        cv::Ptr<Scene> scene(new SVSTiledScene(filePath,"Image",
            image_dirs, tiff));
        scenes.push_back(scene);
    }
    if(thumbnail>=0)
    {
        cv::Ptr<Scene> scene(new SVSSmallScene(filePath,"Thumbnail",
            directories[thumbnail], tiff));
        scenes.push_back(scene);
    }
    if(label>=0)
    {
        cv::Ptr<Scene> scene(new SVSSmallScene(filePath,"Label",
            directories[label], tiff));
        scenes.push_back(scene);
    }
    if(macro>=0)
    {
        cv::Ptr<Scene> scene(new SVSSmallScene(filePath,"Macro",
            directories[macro], tiff));
        scenes.push_back(scene);
    }
    slide.reset(new SVSSlide);
    slide->m_Scenes.assign(scenes.begin(), scenes.end());
    slide->m_filePath = filePath;
    
    return slide;
}
