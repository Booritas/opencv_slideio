// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#ifndef OPENCV_slideio_czisubblock_HPP
#define OPENCV_slideio_czisubblock_HPP
#include "opencv2/core.hpp"
#include <cstdint>
#include <vector>
#include "czistructs.hpp"
#include "structs.hpp"

namespace cv
{
    namespace slideio
    {
        class CV_EXPORTS CZISubBlock
        {
        public:
            enum Compression
            {
                Uncompressed = 0,
                Jpeg = 1,
                LZW = 2,
                JpegXR = 4
            };
            CZISubBlock();
            int firstChannel() const { return firstDimensionIndex(m_channelIndex);}
            int lastChannel() const  { return lastDimensionIndex(m_channelIndex); }
            int firstZSlice() const { return firstDimensionIndex(m_zSliceIndex); }
            int lastZSlice() const { return lastDimensionIndex(m_zSliceIndex); }
            int firstTFrame() const { return firstDimensionIndex(m_tFrameIndex); }
            int lastTFrame() const { return lastDimensionIndex(m_tFrameIndex); }
            int firstScene() const { return firstDimensionIndex(m_sceneIndex); }
            int lastScene() const { return lastDimensionIndex(m_sceneIndex); }
            int firstIllumination() const { return firstDimensionIndex(m_illuminationIndex); }
            int lastIllumination() const { return lastDimensionIndex(m_illuminationIndex); }
            int firstRotation() const { return firstDimensionIndex(m_rotationIndex); }
            int lastRotation() const { return lastDimensionIndex(m_rotationIndex); }
            int firstBAccusition() const { return firstDimensionIndex(m_bAccusitionIndex); }
            int lastBAccusition() const { return lastDimensionIndex(m_bAccusitionIndex); }
            int firstHPhase() const { return firstDimensionIndex(m_hPhaseIndex); }
            int lastHPhase() const { return lastDimensionIndex(m_hPhaseIndex); }
            int firstView() const { return firstDimensionIndex(m_viewIndex); }
            int lastView() const { return lastDimensionIndex(m_viewIndex); }
            double zoom() const { return m_zoom; }
            const cv::Rect& rect() const { return m_rect; }
            int cziPixelType() const { return m_cziPixelType; }
            int64_t computeDataOffset(int channel, int z, int t, int r, int s, int i, int b, int h, int v) const;
            void setupBlock(const SubBlockHeader& subblockHeader, std::vector<DimensionEntryDV>& dimensions);
            bool isInBlock(int channel, int z, int t, int r, int s, int i, int b, int h, int v) const;
            int pixelSize() const { return m_pixelSize; }
            slideio::DataType dataType() const {return m_dataType;};
            int planeSize() const {return m_planeSize;}
            uint64_t dataPosition() const {return m_dataPosition;}
            uint64_t dataSize() const {return m_dataSize;}
            Compression compression() const {return static_cast<Compression>(m_compression);}
            const std::vector<Dimension>& dimensions() const {return m_dimensions;}
        private:
            int firstDimensionIndex(int dimension) const
            {
                if(dimension>=0 && dimension<static_cast<int>(m_dimensions.size()))
                    return m_dimensions[dimension].start;
                return 0;
            }
            int lastDimensionIndex(int dimension) const
            {
                if (dimension >= 0 && dimension < static_cast<int>(m_dimensions.size()))
                    return  (m_dimensions[dimension].start + m_dimensions[dimension].size - 1);
                return 0;
            }
        private:
            slideio::DataType m_dataType;
            cv::Rect m_rect;
            int32_t m_cziPixelType;
            int32_t m_pixelSize;
            int32_t m_planeSize;
            int64_t m_filePosition;
            int64_t m_dataPosition;
            int64_t m_dataSize;
            int32_t m_filePart;
            int32_t m_compression;
            int m_channelIndex;
            int m_zSliceIndex;
            int m_tFrameIndex;
            int m_illuminationIndex;
            int m_bAccusitionIndex;
            int m_rotationIndex;
            int m_sceneIndex;
            int m_hPhaseIndex;
            int m_viewIndex;
            double m_zoom;
            std::vector<Dimension> m_dimensions;
        };
        typedef std::vector<CZISubBlock> CZISubBlocks;
    }
}
#endif