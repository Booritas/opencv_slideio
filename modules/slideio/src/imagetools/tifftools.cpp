// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/tifftools.hpp"
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/slideio.hpp"
#include "opencv2/core.hpp"
#include <boost/format.hpp>

using namespace cv;

void  slideio::TiffTools::scanTiffDirTags(TIFF* tiff, slideio::TiffDirectory& dir)
{
    char *description(nullptr);
    short dirchnls(0), dirbits(0);
    uint16_t compress(0);
    short  planar_config(0);
    int width(0), height(0), tile_width(0), tile_height(0);
    TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &dirchnls);
    TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &dirbits);
    TIFFGetField(tiff, TIFFTAG_COMPRESSION, &compress);
    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tiff,TIFFTAG_TILEWIDTH ,&tile_width);
    TIFFGetField(tiff,TIFFTAG_TILELENGTH,&tile_height);
    TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &description);
    TIFFGetField(tiff, TIFFTAG_PLANARCONFIG ,&planar_config);	
    float resx(0), resy(0);
    uint16 units(0);
    TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &resx);
    TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &resy);
    TIFFGetField(tiff, TIFFTAG_RESOLUTIONUNIT, &units);
    dir.interleaved = planar_config==PLANARCONFIG_CONTIG;
    float posx(0), posy(0);
    TIFFGetField(tiff, TIFFTAG_XPOSITION, &posx);
    TIFFGetField(tiff, TIFFTAG_YPOSITION, &posy);
    int32 rowsPerStripe(0);
    TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &rowsPerStripe);
    TIFFDataType dt(TIFF_NOTYPE);
    TIFFGetField(tiff, TIFFTAG_DATATYPE, &dt);
    dir.stripSize = (int)TIFFStripSize(tiff);
    dir.dataType = dt;
    if(units==RESUNIT_INCH && resx>0 && resy>0){
        dir.res.x = 0.01/resx;
        dir.res.y = 0.01/resy;
    }
    else if(units==RESUNIT_INCH && resx>0 && resy>0){
        dir.res.x = 0.0254/resx;
        dir.res.y = 0.0254/resy;
    }
    else{
        dir.res.x = 0.;
        dir.res.y = 0.;
    }
    dir.position = {posx, posy};
    bool tiled = TIFFIsTiled(tiff);
    dir.description = description;
    dir.bitsPerSample = dirbits;
    dir.channels = dirchnls;
    dir.height = height;
    dir.width = width;
    dir.tileHeight = tile_height;
    dir.tileWidth = tile_width;
    dir.tiled = tiled;
    dir.compression = compress;
    dir.rowsPerStrip = rowsPerStripe;
    
    
}

void slideio::TiffTools::scanTiffDir(TIFF* tiff, slideio::TiffDirectory& dir)
{
    scanTiffDirTags(tiff, dir);
    dir.offset = 0;
    long subdirs(0);
    int64 *offsets_raw(nullptr);
    if(TIFFGetField(tiff, TIFFTAG_SUBIFD, &subdirs, &offsets_raw))
    {
        std::vector<int64> offsets(offsets_raw, offsets_raw+subdirs);
        if(subdirs>0)
        {
            dir.subdirectories.resize(subdirs);
        }
        for(int subdir=0; subdir<subdirs; subdir++)
        {
            if(TIFFSetSubDirectory(tiff, offsets[subdir]))
            {
                dir.subdirectories[subdir].offset = offsets[subdir];
                scanTiffDirTags(tiff, dir.subdirectories[subdir]);	
            }
        }
    }
}

void slideio::TiffTools::scanFile(TIFF* tiff, std::vector<TiffDirectory>& directories)
{
    int dirs = TIFFNumberOfDirectories(tiff);
    directories.resize(dirs);
    for(int dir=0; dir<dirs; dir++)
    {
        TIFFSetDirectory(tiff,static_cast<uint16_t>(dir));
        directories[dir].dirIndex = dir;
        scanTiffDir(tiff, directories[dir]);
    }	
}

void slideio::TiffTools::scanFile(const std::string& filePath, std::vector<TiffDirectory>& directories)
{
    TIFF* file(nullptr);
    try
    {
        file = TIFFOpen(filePath.c_str(), "r");
        if(file==nullptr)
            throw std::runtime_error(std::string("TiffTools: cannot open tiff file") + filePath);
        scanFile(file, directories);
    }
    catch(std::exception& ex)
    {
        if(file)
            TIFFClose(file);
        throw ex;
    }
    if(file)
        TIFFClose(file);
}

int slideio::TiffTools::dataTypeSize(TIFFDataType dt)
{
    switch(dt)
    {
    case TIFF_LONG8:
    case TIFF_BYTE:
    case TIFF_SBYTE:
    case TIFF_SLONG8:
        return 1;
    case TIFF_SHORT:
    case TIFF_SSHORT:
        return 2;
    case TIFF_LONG:
    case TIFF_SLONG:
        return 4;
    case TIFF_FLOAT:
        return 4;
    case TIFF_DOUBLE:
        return 8;
    }
    return 0;
}

slideio::DataType slideio::TiffTools::dataTypeFromTIFFDataType(TIFFDataType dt)
{
    switch(dt)
    {
    case TIFF_NOTYPE:
        return DataType::DT_None;
    case TIFF_LONG8:
    case TIFF_BYTE:
        return DataType::DT_Byte;
    case TIFF_ASCII:
        return DataType::DT_None;
    case TIFF_SHORT:
        return DataType::DT_UInt16;
    case TIFF_SLONG8:
    case TIFF_SBYTE:
        return DataType::DT_Int8;
    case TIFF_UNDEFINED:
        return DataType::DT_Unknown;
    case TIFF_SSHORT:
        return DataType::DT_Int16;
    case TIFF_SRATIONAL:
        return DataType::DT_Unknown;
    case TIFF_FLOAT:
        return DataType::DT_Float32;
    case TIFF_DOUBLE:
        return DataType::DT_Float64;
    case TIFF_IFD:
    case TIFF_RATIONAL:
    case TIFF_IFD8:
    default: ;
        return DataType::DT_Unknown;
    }
}

void slideio::TiffTools::readStripedDir(TIFF* file, const slideio::TiffDirectory& dir, cv::OutputArray output)
{
    if(!dir.interleaved)
        throw std::runtime_error("Planar striped images are not supported");
    
    int buff_size = dir.width*dir.height*dir.channels*dataTypeSize(dir.dataType);
    cv::Size sizeImage = { dir.width, dir.height };
    slideio::DataType dt = dataTypeFromTIFFDataType(dir.dataType);
    output.create(sizeImage, CV_MAKETYPE(slideio::toOpencvType(dt), dir.channels));
    cv::Mat imageRaster = output.getMat();
    TIFFSetDirectory(file, static_cast<uint16_t>(dir.dirIndex));
    if(dir.offset>0){
        TIFFSetSubDirectory(file, dir.offset);
    }
    uint8* buff_begin = imageRaster.data;
    int strip_buf_size = dir.stripSize;
    
    for(int strip=0, row=0; row<dir.height; strip++, row+=dir.rowsPerStrip, buff_begin+=strip_buf_size)
    {
        if((strip+strip_buf_size)>buff_size)
            strip_buf_size = buff_size - strip;
        int read = (int)TIFFReadEncodedStrip(file, strip, buff_begin, strip_buf_size);
        if(read<=0){
            throw std::runtime_error("TiffTools: Error by reading of tif strip");
        }
    }
    return;
}


void slideio::TiffTools::readTile(TIFF* hFile, const slideio::TiffDirectory& dir, int tile,
    const std::vector<int>& channelIndices, cv::OutputArray output)
{
    if(!dir.tiled){
        throw std::runtime_error("TiffTools: Expected tiled configuration, received striped");
    }
    setCurrentDirectory(hFile, dir);

    if(dir.compression==34712 || dir.compression==33003)
    {
        readJ2KTile(hFile, dir, tile, channelIndices, output);
    }
    else
    {
        readRegularTile(hFile, dir, tile, channelIndices, output);
    }
}

void slideio::TiffTools::readRegularTile(TIFF* hFile, const slideio::TiffDirectory& dir, int tile,
            const std::vector<int>& channelIndices, cv::OutputArray output)
{
    cv::Size tileSize = { dir.tileWidth, dir.tileHeight };
    slideio::DataType dt = dataTypeFromTIFFDataType(dir.dataType);
    cv::Mat tileRaster;
    tileRaster.create(tileSize, CV_MAKETYPE(slideio::toOpencvType(dt), dir.channels));
    TIFFSetDirectory(hFile, static_cast<uint16_t>(dir.dirIndex));
    if (dir.offset > 0) {
        TIFFSetSubDirectory(hFile, dir.offset);
    }
    uint8* buff_begin = tileRaster.data;
    auto buf_size = tileRaster.total()*tileRaster.elemSize();
    auto readBytes = TIFFReadEncodedTile(hFile, tile, buff_begin, buf_size);
    if(readBytes<=0)
        throw std::runtime_error(
        (boost::format(
            "TiffTools: error reading endoced tiff tile %1% of directory %2%."
            "Compression: %3%") % tile %dir.dirIndex % dir.compression).str());
    if(channelIndices.empty())
    {
        tileRaster.copyTo(output);
    }
    else if(channelIndices.size()==1)
    {
        cv::extractChannel(tileRaster, output, channelIndices[0]);
    }
    else
    {
        std::vector<cv::Mat> channelRasters;
        channelRasters.reserve(channelIndices.size());
        for(int channelIndex : channelIndices)
        {
            cv::Mat channelRaster;
            cv::extractChannel(tileRaster, channelRaster, channelIndices[channelIndex]);
        }
        cv::merge(channelRasters, output);
    }
}

void slideio::TiffTools::readJ2KTile(TIFF* hFile, const slideio::TiffDirectory& dir, int tile,
            const std::vector<int>& channelIndices, cv::OutputArray output)
{
    const auto tileSize = TIFFTileSize(hFile);
    std::vector<uint8_t> rawTile(tileSize);
    if(dir.interleaved)
    {
        // process interleaved channels
        tmsize_t readBytes = TIFFReadRawTile(hFile, tile, rawTile.data(), (int)rawTile.size());
        if(readBytes<=0){
            throw std::runtime_error("TiffTools: Error reading raw tile");
        }
        bool yuv = dir.compression==33003;
        slideio::ImageTools::decodeJp2KStream(rawTile, output, channelIndices, yuv);
    }
    else if(channelIndices.size()==1)
    {
        // process a single planar channel
        throw std::runtime_error("Not implemented");
    }
    else
    {
        throw std::runtime_error("Not implemented");
        //// process planar channels
        //std::vector<cv::Mat> channelRasters;
        //for(const auto& channelIndex : channelIndices)
        //{
        //    
        //}
    }
}


void slideio::TiffTools::setCurrentDirectory(TIFF* hFile, const slideio::TiffDirectory& dir)
{
    if(!TIFFSetDirectory(hFile, static_cast<uint16_t>(dir.dirIndex))){
        throw std::runtime_error("TiffTools: error by setting current directory");
    }
    if(dir.offset>0){
        if(!TIFFSetSubDirectory(hFile, dir.offset)){
            throw std::runtime_error("TiffTools: error by setting current sub-directory");
        }
    }
}

