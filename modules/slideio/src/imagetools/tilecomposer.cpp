// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/tilecomposer.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/slideio/imagetools.hpp"


using namespace cv;


void slideio::TileComposer::composeRect(slideio::Tiler* tiler,
                                        const std::vector<int>& channelIndices,
                                        const cv::Rect& blockRect,
                                        const cv::Size& blockSize,
                                        cv::OutputArray output,
                                        void *userData)
{
    const int tileCount = tiler->getTileCount(userData);
    const int channelCount = static_cast<int>(channelIndices.size());
    cv::Mat scaledBlockRaster;
    const cv::Point blockOrigin = blockRect.tl();
    const double scaleX = static_cast<double>(blockSize.width)/static_cast<double>(blockRect.width);
    const double scaleY = static_cast<double>(blockSize.height)/static_cast<double>(blockRect.height);
    cv::Rect scaledBlockRect;
    slideio::ImageTools::scaleRect(blockRect, blockSize, scaledBlockRect);

    for(int tileIndex = 0; tileIndex<tileCount; tileIndex++)
    {
        cv::Rect tileRect;
        tiler->getTileRect(tileIndex, tileRect, userData);
        cv::Rect intersection = blockRect & tileRect;
        if(intersection.area()>0)
        {
            cv::Mat tileRaster;
            if(tiler->readTile(tileIndex, channelIndices, tileRaster, userData))
            {
                if(scaledBlockRaster.empty())
                {
                    output.create(scaledBlockRect.height, scaledBlockRect.width, tileRaster.type());
                    scaledBlockRaster = output.getMat();
                }
                cv::Rect scaledTileRect;
                slideio::ImageTools::scaleRect(tileRect, scaleX, scaleY, scaledTileRect);
                // scale tile raster
                cv::Mat scaledTileRaster;
                cv::resize(tileRaster, scaledTileRaster, scaledTileRect.size());
                // compute intersection of scaled tile rectangle and scaled block rectangle
                cv::Rect scaledIntersectionRect = scaledBlockRect & scaledTileRect;
                const cv::Rect blockPart = scaledIntersectionRect - scaledBlockRect.tl();
                const cv::Rect tilePart = scaledIntersectionRect - scaledTileRect.tl();
                cv::Mat blockPartRaster(scaledBlockRaster, blockPart);
                cv::Mat tilePartRaster(scaledTileRaster, tilePart);
                tilePartRaster.copyTo(blockPartRaster);
            }
        }
    }
}
