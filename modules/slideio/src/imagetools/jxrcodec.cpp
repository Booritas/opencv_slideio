// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/imagetools.hpp"
#include "opencv2/slideio.hpp"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <JXRGlue.h>
#include <JXRTest.h>

using namespace cv;

template <class T>
class JxrObjectKeeper
{
public:
    JxrObjectKeeper(T *object=nullptr) : m_object(object)
    {
        
    }
    ~JxrObjectKeeper()
    {
        if(m_object)
        {
            m_object->Release(&m_object);
        }
    }
    operator T*&()
    {
        return m_object;
    }
    T*& object()
    {
        return m_object;
    }
    T* operator->()
    {
        return m_object;
    }
    T* m_object;
};

void slideio::ImageTools::readJxrImage(const std::string& path, cv::OutputArray output)
{
    namespace fs = boost::filesystem;
    if(!fs::exists(path))
    {
        throw std::runtime_error(
            (boost::format("File %1% does not exist") % path).str()
        );
    }
    JxrObjectKeeper<PKCodecFactory> codecFactory(nullptr);
    PKCreateCodecFactory(&codecFactory.object(), WMP_SDK_VERSION);
    JxrObjectKeeper<PKImageDecode> decoder(nullptr);
    ERR err = codecFactory->CreateDecoderFromFile(path.c_str(), &decoder.object());
    if(err!=WMP_errSuccess)
    {
        throw std::runtime_error(
            (boost::format("Cannot create decoder from file. Error code: %1%") % err).str());
    }
    U32 numFrames = 0;
    decoder->GetFrameCount(decoder, &numFrames);
    if(numFrames>1)
    {
        throw std::runtime_error(
            (boost::format("JxrDecoder: unexpected number of sub-images: %1%") % numFrames).str());
    }
    PKPixelInfo pixelFormatInfo;
    pixelFormatInfo.pGUIDPixFmt = &decoder->guidPixFormat;
    err = PixelFormatLookup(&pixelFormatInfo, LOOKUP_FORWARD);
    if(err!=WMP_errSuccess)
    {
        throw std::runtime_error("Unsupported pixel format");
    }
    const int numChannels = pixelFormatInfo.cChannel;
    int dataSize = (pixelFormatInfo.uBitsPerSample-1)/8 + 1;

    JxrObjectKeeper<PKFactory> factory(nullptr);
    PKCreateFactory(&factory.object(), PK_SDK_VERSION);

    decoder->WMP.wmiI.cROILeftX = 0;
    decoder->WMP.wmiI.cROITopY = 0;
    decoder->WMP.wmiI.cROIWidth = decoder->WMP.wmiI.cWidth;
    decoder->WMP.wmiI.cROIHeight = decoder->WMP.wmiI.cHeight;


    output.create(decoder->uHeight, decoder->uWidth, CV_MAKETYPE(CV_8U, 3));
    cv::Mat raster = output.getMat();
    char* outputFileExtension = ".bmp";

    Float rX = 0.0, rY = 0.0;
    PKRect rect = {0, 0, 0, 0};
    rect.Width = (I32)(decoder->WMP.wmiI.cROIWidth);
    rect.Height = (I32)(decoder->WMP.wmiI.cROIHeight);
    decoder->GetResolution(decoder.object(), &rX, &rY);
    const int rasterSize = rect.Height*rect.Width*numChannels;
    const int size = rasterSize + 100;
    std::vector<uint8_t> buff(size);
    const auto outFormat = GUID_PKPixelFormat24bppRGB;
    struct WMPStream* encodeStream = nullptr;
    JxrObjectKeeper<PKFormatConverter> converter(nullptr);
    codecFactory->CreateFormatConverter(&converter.object());
    err = converter->Initialize(converter, decoder.object(), outputFileExtension, outFormat);
    if(err!=WMP_errSuccess)
    {
        throw std::runtime_error(
            (boost::format("Error by initialization of format converter: %1%") % err).str());
    }

    factory->CreateStreamFromMemory(&encodeStream, buff.data(), size);
    const PKIID* encoderIID(nullptr);
    GetTestEncodeIID(outputFileExtension, &encoderIID);

    JxrObjectKeeper<PKImageEncode> encoder(nullptr);
    PKTestFactory_CreateCodec(encoderIID,  (void**)&encoder.object());
    encoder->Initialize(encoder, encodeStream, nullptr, 0);
    encoder->SetPixelFormat(encoder, outFormat);
    encoder->SetResolution(encoder, rX, rY);
    encoder->WMP.wmiSCP.bBlackWhite = decoder->WMP.wmiSCP.bBlackWhite;
    encoder->SetSize(encoder, rect.Width, rect.Height);
    encoder->WriteSource = PKImageEncode_Transcode;
    encoder->WriteSource(encoder, converter, &rect);

    const size_t offset = encoder->offPixel;
    std::copy(buff.data() + offset, buff.data()+offset+rasterSize, raster.data);

    cv::flip(raster, raster, 0);
}
