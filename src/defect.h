/*
 * Copyright (c) 2009 Javier G. Orlandi <dherkova@gmail.com>
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

#ifndef _DEFECT_H_
#define _DEFECT_H_

#include <vector>
#include <Eigen/Core>
#include "neuronnamespace.h"

enum defectType { DEFECT_TYPE_DISK, DEFECT_TYPE_RECTANGLE, DEFECT_TYPE_CHAIN, DEFECT_TYPE_PIXEL, DEFECT_TYPE_SEGMENT };
enum defectClassIndex
{
    DEFECT_CLASS_UNDEFINED = 0x00,
    DEFECT_CLASS_SOMA = 0x01,
    DEFECT_CLASS_AXON = 0x02,
    DEFECT_CLASS_PATTERN = 0x04,
    DEFECT_CLASS_BOUNDARY = 0x08,
    DEFECT_CLASS_DTREE = 0x16
};

// import most common Eigen types 
//USING_PART_OF_NAMESPACE_EIGEN

using namespace Eigen;

class Defect
{
	public:
        Defect();
        Defect(defectType typ, defectClassIndex clas, int overl, std::vector<double> siz, std::vector<Vector2d > poi, 
               int idx = 0);
        std::vector<Vector2d> getDefectLimits();
        inline std::vector<Vector2d> getPoints()
            {return points;}
        inline void setPoints(std::vector<Vector2d> poi)
            {points = poi;}
        bool intersect(Defect newDefect, int boundaries = 0, double w = 0., double h = 0.);
        inline int getOverlapType()
            {return overlap;}
        inline int getClassType()
            {return classType;}
        inline int getIndex()
            {return index;}
        inline void setIndex(int idx)
            {index = idx;}
        inline int getDefectType()
            {return type;}
        inline std::vector<double> getSizes()
            {return sizes;}
    private:
        int type, overlap, classType, index;
        std::vector<double> sizes;
        std::vector<Vector2d> points;
};

bool operator == (Defect& left, Defect& right);
bool operator != (Defect& left, Defect& right);

inline bool operator == (Defect& left, Defect& right)
{
    if(left.getPoints() == right.getPoints())
        return true;
    else
        return false;
}

inline bool operator != (Defect& left, Defect& right)
{
    return !(left == right);
}

#endif
    // _DEFECT_H_

