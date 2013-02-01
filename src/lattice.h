/*
 * Copyright (c) 2009-2013 Javier G. Orlandi <javiergorlandi@gmail.com>
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

#ifndef _LATTICE_H_
#define _LATTICE_H_

#include <Eigen/Core>
#include <list>
#include <vector>
#include "neuronnamespace.h"

// import most common Eigen types 
//USING_PART_OF_NAMESPACE_EIGEN
using namespace Eigen;

class Defect;

// For now square lattices - although generalization shouldn't be hard
class Lattice
{
    public:
        Lattice();
        Lattice(int boundaries, Vector2d orig, double unitWidth, double unitHeight, double wid, double hei);
        bool addDefect(Defect def);
        bool firstBoundaryOverflow(Vector2d point);
        bool secondBoundaryOverflow(Vector2d point);
        inline int getBoundaryConditions()
            {return boundaryConditions;}
        inline double getWidth()
            {return widthCount*e1.norm();}
        inline double getHeight()
            {return heightCount*e2.norm();}
        Vector2i getClosestLatticePoint(Vector2d point);
        Vector2d fromAbsoluteToPeriodic(Vector2d point);
        std::list<Defect> getDefectsInRange(std::vector<Vector2d> bounds);
       inline std::list<Defect> getAllDefects()
           {return defectList;}

    private:
        int boundaryConditions;
        // origin in absolute cartesians, {e1,e2} basis of the lattice
        Vector2d origin, e1, e2;
        int widthCount, heightCount;
        std::list<Defect> defectList;
        std::vector<Defect*> **defectPosition;
        
};

#endif
    // _LATTICE_H_

