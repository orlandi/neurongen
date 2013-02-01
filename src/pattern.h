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

#ifndef _PATTERN_H_
#define _PATTERN_H_

#include <Eigen/Core>
#include "neuronnamespace.h"

enum patternDrawMode { PATTERN_DRAW_MODE_CONTOUR, PATTERN_DRAW_MODE_FILL, PATTERN_DRAW_MODE_3D };

//USING_PART_OF_NAMESPACE_EIGEN

using namespace Eigen;

class Pattern
{
    public:
        Pattern(double x, double y, double wSize, double hSize);
        Pattern(double wSize, double hSize);

        void loadPatternFromFile(std::string file);
        void createEmptyPattern(size_t wCount, size_t hCount);

        inline Vector2d getOrigin()
            {return origin;}
        inline bool checkPattern(int x, int y)
            {return pattern[x][y];}
        inline Vector2d getSize()
            {return Vector2d(width, height);}
        inline Vector2d getUnitSize()
            {return unitSize;}
        inline Vector2i getSizeCount()
            {return Vector2i(int(widthCount), int(heightCount));}
        Vector2d getPosition(int x, int y);
        inline bool** getPattern()
            {return pattern;}

    private:
        void init();
        int drawMode;

        bool **pattern;
        size_t widthCount, heightCount;
        Vector2d origin, unitSize;
        double width, height;
        std::string fileName;
        float backgroundColor[3], patternColor[3];
};

#endif
    // _PATTERN_H_

