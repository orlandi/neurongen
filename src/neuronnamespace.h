/*
 * Copyright (c) 2009-2010 Javier G. Orlandi <orlandi@dherkova.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _NEURONNAMESPACE_H_
#define _NEURONNAMESPACE_H_

#include <string>
#include <sstream>

namespace neuron
{
    enum chamberType { CH_TYPE_CIRCULAR, CH_TYPE_RECTANGULAR, CH_TYPE_CUSTOM, CH_TYPE_CUSTOM_WITH_DENSITY_MAP};
    enum units { UNITS_CENTIMETERS, UNITS_MILIMETERS, UNITS_MICROMETERS, UNITS_NANOMETERS,
                 UNITS_PERCM2, UNITS_PERMM2, UNITS_PERUM2,
                 UNITS_DEGREES, UNITS_RADIANS };
    enum chamberDimensions { CH_DIMENSIONS_ABSOLUTE, CH_DIMENSIONS_PERPIXEL };
    enum displayLists { DL_CHAMBER = 2005, DL_PATTERN = 2105, DL_NEURONS = 2205};
    enum distribution { DISTRIBUTION_DELTA, DISTRIBUTION_RAYLEIGH, DISTRIBUTION_UNIFORM,
                        DISTRIBUTION_GAUSSIAN, DISTRIBUTION_CUX };
    enum neuronAmountType { NEU_AMOUNT_NUMBER, NEU_AMOUNT_DENSITY };
    enum somaShape { SOMA_SHAPE_CIRCULAR };
    enum axonType { AXON_TYPE_STRAIGHT, AXON_TYPE_SEGMENTED };
    enum axonSegmentType { AXON_STYPE_FIXEDNUMBER, AXON_STYPE_FIXEDLENGTH };
    enum dTreeType { DTREE_TYPE_HOMOGENEOUS, DTREE_TYPE_FRACTAL };
    enum dTreeShape { DTREE_SHAPE_CIRCULAR, DTREE_SHAPE_CONICAL };
    enum dTreeSizeDistributionType { DTREE_SIZE_DISTTYPE_DELTA, DTREE_SIZE_DISTTYPE_RAYLEIGH };
    enum latticeBoundaries { LATTICE_BOUNDARIES_REFLECTIVE, LATTICE_BOUNDARIES_ABSORBENT, LATTICE_BOUNDARIES_PERIODIC};
    enum collisionFLag
    {
        COL_SELF = 0x01,
        COL_PATTERN = 0x02,
        COL_BOUNDARIES = 0x04,
        COL_SOMAS = 0x08,
        COL_AXONS = 0x0F
    };

    enum neuronDrawFlag
    {
        DRAW_SOMA = 0x01,
        DRAW_AXON = 0x02,
        DRAW_DTREE = 0x04,
        DRAW_CON_INPUT = 0x08,
        DRAW_CON_OUTPUT = 0x10,
        DRAW_KCORE = 0x20
        //0x40 0x80
    };

    typedef struct chamberParameters
    {
        bool pattern;
        int type, units, dimensions;
        double width, height, radius;
        std::string patternFile;
        std::string print()
        {
            std::stringstream tmpStr;
            tmpStr << "Chamber parameters:\n"
                   << "Type: " << type << "\n"
                   << "Units: " << units << "\n"
                   << "Dimensions: " << dimensions << "\n"
                   << "Width: " << width << " Height: " << height << " Radius: " << radius << "\n"
                   << "Pattern: " << pattern << " File: " << patternFile << "\n\n";
            return tmpStr.str();
        };
    } chamberParameters;
    // Temporary solution - need to use enums
    const chamberParameters DEFAULT_CHAMBER_PARAMETERS =
        {false, -1, -1, -1, -1, -1, -1, ""};

    typedef struct somaParameters
    {
        int shape, units;
        double radius;
        int collisionFlags;
    } somaParameters;
    const somaParameters DEFAULT_SOMA_PARAMETERS =
        {SOMA_SHAPE_CIRCULAR, UNITS_MILIMETERS, .001, 0x01 & 0x02 & 0x04 & 0x08};

    typedef struct dtreeParameters
    {
        int type, shape, units;
        int sizeDistribution;
        double meanRadius, stdRadius;
        int collisionFlags;
        bool CUX;
        double CUXfraction, CUXmultiplier;
    } dtreeParameters;
    const dtreeParameters DEFAULT_DTREE_PARAMETERS =
        {DTREE_TYPE_HOMOGENEOUS, DTREE_SHAPE_CIRCULAR, UNITS_MILIMETERS, DISTRIBUTION_GAUSSIAN, .05, .02, 0x00, false, 0, 0}; 

    typedef struct axonParameters
    {
        int type;
        int lengthDistribution, initialAngleDistribution, segmentAngleDistribution, units;
        double meanLength, stdLength, meanInitialAngle, stdInitialAngle, meanSegmentAngle, stdSegmentAngle;
        double width, segmentLength;
        double maxStdSegmentAngle;
        int segmentCount, maxRetries, segmentType;
        int collisionMode, collisionFlags;
    } axonParameters;
    const axonParameters DEFAULT_AXON_PARAMETERS =
        {AXON_TYPE_SEGMENTED, DISTRIBUTION_RAYLEIGH, DISTRIBUTION_UNIFORM, DISTRIBUTION_GAUSSIAN, UNITS_MILIMETERS,
//         0.8, 0., 0., 0., 0., 0.1, 0.001, 0.01, 3.2, 20, 10, AXON_STYPE_FIXEDLENGTH, 0, 0x02 & 0x04};
         0.2, 0., 0., 0., 0., 0.1, 0.001, 0.01, 3.2, 20, 10, AXON_STYPE_FIXEDLENGTH, 0, 0x02 & 0x04};
    typedef struct cultureParameters
    {
        int neuronNumber;
        int placementDistribution;
    } cultureParameters;
    const cultureParameters DEFAULT_CULTURE_PARAMETERS =
        {50, DISTRIBUTION_UNIFORM};
}

#endif
    // _NEURONNAMESPACE_H_

