// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/slideio.hpp"
#include "opencv2/slideio/gdalimagedriver.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/slideio/memory_stream.hpp"

#include <openjpeg.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

using namespace cv;

int slideio::ImageTools::dataTypeSize(slideio::DataType dt)
{
    switch(dt)
    {
    case DataType::DT_Byte:
    case DataType::DT_Int8:
        return 1;
    case DataType::DT_UInt16:
    case DataType::DT_Int16:
    case DataType::DT_Float16:
        return 2;
    case DataType::DT_Int32:
    case DataType::DT_Float32:
        return 4;
    case DataType::DT_Float64:
        return 8;
    }
    throw std::runtime_error(
        (boost::format("Unknown data type: %1%") % (int)dt).str());
}

void slideio::ImageTools::readJp2KFile(const std::string& filePath, cv::OutputArray output)
{
    auto fileSize = boost::filesystem::file_size(filePath);
    if(fileSize<=0)
        throw std::runtime_error(
            (boost::format("Invalid file: %1%") % filePath).str());
    
    std::ifstream file(filePath, std::ios::binary);
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);
    // reserve capacity
    std::vector<uint8_t> vec;
    vec.reserve(fileSize);
    // read the data:
    vec.insert(vec.begin(),
        std::istream_iterator<uint8_t>(file),
        std::istream_iterator<uint8_t>());
    decodeJp2KStream(vec, output);
}

static int getComponentDataType(const opj_image_comp_t* comp)
{
    switch(comp->prec)
    {
    case 8:
        return ((comp->sgnd)?CV_8S:CV_8U);
    case 16:
        return ((comp->sgnd)?CV_16S:CV_16U);
    case 32:
        return CV_32S;
    }
    throw std::runtime_error(
        (boost::format("Unknown data type of data: %1%") % (int)comp->bpp).str());
}

static OPJ_CODEC_FORMAT getJP2KCodec(const std::vector<uint8_t>& data)
{
    static const unsigned char jpc_header[] = {0xff, 0x4f};
    static const unsigned char jp2_box_jp[] = {0x6a, 0x50, 0x20, 0x20}; /* 'jP  ' */

    const uint8_t *buf = data.data();
    size_t len = data.size();

    OPJ_CODEC_FORMAT eCodecFormat;
    if (len >= sizeof(jpc_header) &&
            memcmp(buf, jpc_header, sizeof(jpc_header)) == 0) {
        eCodecFormat = OPJ_CODEC_J2K;
    }
    else if (len >= 4 + sizeof(jp2_box_jp) &&
               memcmp(buf + 4, jp2_box_jp, sizeof(jp2_box_jp)) == 0) {
        eCodecFormat = OPJ_CODEC_JP2;
    }
    else
    {
        throw std::runtime_error("Unknown file format");
    }
    return eCodecFormat;
}

void slideio::ImageTools::decodeJp2KStream(
    const std::vector<uint8_t>& data,
    cv::OutputArray output,
    const std::vector<int>& channelIndices,
    bool forceYUV)
{
    opj_codec_t* codec(nullptr);
    opj_image_t* image(nullptr);
    opj_stream_t* stream(nullptr);
    try
    {
        OPJ_CODEC_FORMAT codecId = getJP2KCodec(data);
        OPJStreamUserData userData((uint8_t*)data.data(), data.size());
        stream = createOPJMemoryStream(&userData, data.size(), true);
        codec = opj_create_decompress(codecId);
        if(!codec)
            throw std::runtime_error("Cannot get required codec");
        opj_dparameters_t jp2dParams;
        opj_set_default_decoder_parameters(&jp2dParams);
        if (!opj_setup_decoder(codec, &jp2dParams)){
            throw std::runtime_error("Cannot setup codec");
        }
        if(!opj_read_header(stream, codec, &image) || (image->numcomps == 0)){
            throw std::runtime_error("Error reading image header");
        }
        if(forceYUV)
            image->color_space = OPJ_CLRSPC_SYCC;
        // decode the image
        OPJ_BOOL ret = opj_decode(codec, stream, image);
        if(!ret)
            throw std::runtime_error("Error by decoding of Jp2K stream");

        opj_end_decompress(codec,stream);
        opj_destroy_codec(codec);
        codec=nullptr;
        opj_stream_destroy(stream);
        stream = nullptr;

        const OPJ_UINT32 imageWidth = image->x1 - image->x0;
        const OPJ_UINT32 imageHeight = image->y1 - image->y0;
        const OPJ_UINT32 numComps = image->numcomps;
        const int dt = getComponentDataType(image->comps);

        output.create(imageHeight, imageWidth,CV_MAKETYPE(dt, image->numcomps));
        const cv::Size imageSize(imageWidth, imageHeight); 

        std::vector<cv::Mat> imagePlanes;
        std::vector<int> channels(channelIndices);

        for(OPJ_UINT32 channel=0; channel<numComps; channel++)
        {
            const opj_image_comp& component = image->comps[channel];
            // create a cv::Mat object with the buffer 
            cv::Mat compRaster32S(component.h, component.w, CV_MAKETYPE(CV_32S, 1), component.data);
            // convert raster from 32 bit integer to the original type
            cv::Mat compRaster;
            compRaster32S.convertTo(compRaster, CV_MAKETYPE(dt,1));
            // check if we need to resize the component
            if(component.w!=imageWidth || component.h!=imageHeight)
            {
                // resize the component so it fits to the image size
                cv::Mat resized(imageSize.height, imageSize.width, CV_MAKETYPE(dt, 1));
                cv::resize(compRaster, resized, imageSize);
                imagePlanes.push_back(resized);
            }
            else
            {
                imagePlanes.push_back(compRaster);
            }
        }
        cv::Mat targetImage;
        if(forceYUV)
        {
            cv::Mat cvImage;
            cv::merge(imagePlanes, cvImage);
            cv::cvtColor(cvImage, targetImage, cv::COLOR_YUV2RGB);
        }
        else
        {
            cv::merge(imagePlanes, targetImage);
        }
        if(channels.empty())
        {
            // if no channel is defined - return all channels
            targetImage.copyTo(output);
        }
        else
        {
            std::vector<cv::Mat> targetChannels;
            for(const int& channel : channels)
            {
                cv::Mat channelRaster;
                cv::extractChannel(targetImage,channelRaster, channel);
                targetChannels.push_back(channelRaster);
            }
            if(targetChannels.size()==1)
            {
                targetChannels[0].copyTo(output);
            }
            else
            {
                cv::merge(targetChannels, output);
            }
        }
        opj_image_destroy(image);
    }
    catch(std::exception& ex)
    {
        if(codec)
            opj_destroy_codec(codec);
        if(image)
            opj_image_destroy(image);
        if(stream)
            opj_stream_destroy(stream);
        throw ex;
    }
}

slideio::DataType slideio::ImageTools::dataTypeFromGDALDataType(GDALDataType dt)
{
    switch(dt)
    {
    case GDT_Unknown:
        return slideio::DataType::DT_Unknown;
    case GDT_Byte:
        return slideio::DataType::DT_Byte;
    case GDT_UInt16:
        return slideio::DataType::DT_UInt16;
    case GDT_Int16:
        return slideio::DataType::DT_Int16;
    case GDT_Int32:
        return slideio::DataType::DT_Int32;
    case GDT_Float32:
        return slideio::DataType::DT_Float32;
    case GDT_Float64:
        return slideio::DataType::DT_Float64;
    default:
        return slideio::DataType::DT_Unknown;
    }

}

void slideio::ImageTools::readGDALImage(const std::string& filePath, cv::OutputArray output)
{
    //
    // read extracted page by GDAL library
    // 
    GDALAllRegister();
    GDALDatasetH hFile = GDALOpen(filePath.c_str(), GA_ReadOnly);
    try
    {
        if(hFile==nullptr)
            throw std::runtime_error(
                (boost::format("Cannot open file: %1% with GDAL") % filePath).str());
        const cv::Size imageSize = {GDALGetRasterXSize(hFile),GDALGetRasterYSize(hFile)};
        const int numChannels = GDALGetRasterCount(hFile);
        std::vector<cv::Mat> channels(numChannels);
        for(int channelIndex=0; channelIndex<numChannels; channelIndex++)
        {
            GDALRasterBandH hBand = GDALGetRasterBand(hFile, channelIndex+1);
            if(hBand==nullptr)
                throw std::runtime_error(
                    (boost::format("Cannot open raster band from: %1%") % filePath).str());
            const GDALDataType dt = GDALGetRasterDataType(hBand);
            const DataType dataType = dataTypeFromGDALDataType(dt);
            if(isValidDataType(dataType)){
                
            }
            const int cvDt = toOpencvType(dataType);
            cv::Mat& channel = channels[channelIndex];
            int channelType = CV_MAKETYPE(cvDt, 1);
            channel.create(imageSize, channelType);
            CPLErr err = GDALRasterIO(hBand, GF_Read,
                0, 0,
                imageSize.width, imageSize.height,
                channel.data,
                imageSize.width, imageSize.height, 
                GDALGetRasterDataType(hBand),0,0);
            if(err!=CE_None)
                throw std::runtime_error(
                    (boost::format("Cannot read raster band from: %1%") % filePath).str());
        }
        cv::merge(channels, output);
        GDALClose(hFile);
    }
    catch(std::exception& exp)
    {
        if(hFile!=nullptr)
            GDALClose(hFile);
        throw exp;
    }
}

void slideio::ImageTools::scaleRect(const cv::Rect& srcRect, const cv::Size& newSize, cv::Rect& trgRect)
{
    double scaleX = static_cast<double>(newSize.width) / static_cast<double>(srcRect.width);
    double scaleY = static_cast<double>(newSize.height) / static_cast<double>(srcRect.height);
    trgRect.x = static_cast<int>(std::floor(static_cast<double>(srcRect.x)*scaleX));
    trgRect.y = static_cast<int>(std::floor(static_cast<double>(srcRect.y)*scaleY));
    trgRect.width = newSize.width;
    trgRect.height = newSize.height;
}

void slideio::ImageTools::scaleRect(const cv::Rect& srcRect, double scaleX, double scaleY, cv::Rect& trgRect)
{
    trgRect.x = static_cast<int>(std::floor(static_cast<double>(srcRect.x)*scaleX));
    trgRect.y = static_cast<int>(std::floor(static_cast<double>(srcRect.y)*scaleY));
    int xn = srcRect.x + srcRect.width;
    int yn = srcRect.y + srcRect.height;
    int dxn = static_cast<int>(std::ceil(static_cast<double>(xn)* scaleX));
    int dyn = static_cast<int>(std::ceil(static_cast<double>(yn)* scaleY));
    trgRect.width = dxn - trgRect.x;
    trgRect.height = dyn - trgRect.y;
}