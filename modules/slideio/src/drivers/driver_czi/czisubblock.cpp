﻿// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
#include "opencv2/slideio/czisubblock.hpp"
#include "opencv2/slideio/cziscene.hpp"
#include <boost/format.hpp>


bool cv::slideio::CZISubBlock::isInBlock(int channel, int z, int t, int r, int s, int i, int b, int h, int v) const
{
    const bool inBlock = (channel >= firstChannel() && channel <= lastChannel()) &&
        (z >= firstZSlice() && z <= lastZSlice()) &&
        (t >= firstTFrame() && t <= lastTFrame()) &&
        (r >= firstRotation() && r <= lastRotation()) &&
        (s >= firstScene() && s <= lastScene()) &&
        (i >= firstIllumination() && s <= lastIlluminarion()) &&
        (b >= firstBAccusition() && b <= lastBAccusition()) &&
        (h >= firstHPhase() && h <= lastHPhase()) &&
        (v >= firstView() && h <= lastView());
    return inBlock;
}

cv::slideio::CZISubBlock::CZISubBlock() : m_pixelType(0), m_pixelSize(1), m_planeSize(0), m_filePosition(-1),
                                         m_filePart(-1), m_compression(-1),
                                         m_sceneId(0), m_channelIndex(0), m_zSliceIndex(-1), m_tFrameIndex(-1),
                                         m_illuminationIndex(-1),
                                         m_bAccusitionIndex(-1), m_rotationIndex(-1), m_sceneIndex(-1),
                                         m_hPhaseIndex(-1), m_viewIndex(-1), m_zoom(1.)
{
}

int64_t cv::slideio::CZISubBlock::computeFileOffset(int channel, int z, int t, int r, int s, int i, int b, int h,
                                                    int v) const
{
    if (!isInBlock(channel, z, t, r, s, i, b, h, v))
        return -1;
    // compute file offset for the channel
    int64_t itemSize = m_planeSize;
    int64_t channelSize(0), zSize(0), tSize(0), rSize(0), sSize(0), iSize(0), bSize(0), hSize(0), vSize(0);
    for (const auto& dim : m_dimensions)
    {
        switch (dim.type)
        {
        case 'C':
            channelSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'T':
            tSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'Z':
            zSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'R':
            rSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'S':
            sSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'I':
            iSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'B':
            bSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'H':
            hSize = itemSize;
            itemSize *= dim.size;
            break;
        case 'V':
            vSize = itemSize;
            itemSize *= dim.size;
            break;
        default:
            throw std::runtime_error(
                (boost::format("CZIImageDriver: Unknown dimension: %1%") % dim.type).str());
        }
    }
    const int64_t offset = m_filePosition +
        (channel - firstChannel()) * channelSize +
        (z - firstZSlice()) * zSize +
        (t - firstTFrame()) * tSize +
        (r - firstRotation()) * rSize +
        (s - firstScene()) * sSize +
        (i - firstIllumination()) * iSize +
        (b - firstBAccusition()) * bSize +
        (h - firstHPhase()) * hSize +
        (v - firstView()) * vSize;
    return offset;
}

void cv::slideio::CZISubBlock::setupBlock(const DirectoryEntryDV& entryHeader, std::vector<DimensionEntryDV>& dimensionEntries)
{
    m_filePosition = entryHeader.filePosition;
    m_compression = entryHeader.compression;
    m_filePart = entryHeader.filePart;
    m_pixelType = entryHeader.pixelType;
    m_dimensions.reserve(entryHeader.dimensionCount);
    DataType dt;
    int numComponents;
    CZIScene::channelComponentInfo(static_cast<CZIDataType>(m_pixelType), dt, numComponents, m_pixelSize);
    m_planeSize = m_pixelSize * m_rect.width * m_rect.height;
    for (int dim = 0; dim < entryHeader.dimensionCount; ++dim)
    {
        const DimensionEntryDV& dimEntry = dimensionEntries[dim];
        if (dimEntry.dimension[0] == 'X')
        {
            m_rect.x = dimEntry.start;
            m_rect.width = dimEntry.storedSize;
            m_zoom = static_cast<double>(dimEntry.storedSize) / static_cast<double>(dimEntry.size);
        }
        else if (dimEntry.dimension[0] == 'Y')
        {
            m_rect.y = dimEntry.start;
            m_rect.height = dimEntry.storedSize;
        }
        else
        {
            if (dimEntry.dimension[0] == 'C')
            {
                m_channelIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'Z')
            {
                m_zSliceIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'T')
            {
                m_tFrameIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'I')
            {
                m_illuminationIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'B')
            {
                m_bAccusitionIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'R')
            {
                m_rotationIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'S')
            {
                m_sceneIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'H')
            {
                m_hPhaseIndex = static_cast<int>(m_dimensions.size());
            }
            else if (dimEntry.dimension[0] == 'V')
            {
                m_viewIndex = static_cast<int>(m_dimensions.size());
            }
            Dimension dimension = { dimEntry.dimension[0], dimEntry.start , dimEntry.size};
            m_dimensions.push_back(dimension);
        }
        m_sceneId = CZIScene::sceneIdFromDims(m_dimensions);
    }
}