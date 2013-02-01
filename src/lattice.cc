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

#include "defect.h"
#include "lattice.h"

Lattice::Lattice()
{

}

Lattice::Lattice(int boundaries, Vector2d orig, double unitWidth, double unitHeight, double wid, double hei)
{
    boundaryConditions = boundaries;
    origin = orig;
    e1 = Vector2d(unitWidth, 0.);
    e2 = Vector2d(0., -unitHeight);
    widthCount = ceil(wid/unitWidth);
    heightCount = ceil(hei/unitHeight);

    if(boundaryConditions == neuron::LATTICE_BOUNDARIES_PERIODIC)
    {
        origin *= 2;
        widthCount *= 2;
        heightCount *= 2;
    }
    // [e1,e2]. NOT [rows,cols]
    defectPosition = new std::vector<Defect*>*[widthCount];
    for (int i = 0; i < widthCount; i++)
        defectPosition[i] = new std::vector<Defect*>[heightCount];
}

// Create a defect and attach it to the associated lattice points
bool Lattice::addDefect(Defect def)
{
//    std::cout << "chu\n";
    std::vector<Vector2d> points, newPoints;
/*    if(boundaryConditions == LATTICE_BOUNDARIES_PERIODIC)
    {
        points = def.getPoints();
        for(std::vector<Vector2d>::iterator i = points.begin(); i != points.end(); i++)
        {
            newPoints.push_back(fromAbsoluteToPeriodic(*i));
        }
        def.setPoints(newPoints);
    }*/

    defectList.push_back(def);
    Defect* ref = &defectList.back();

    std::vector<Vector2d> limits = ref->getDefectLimits();
    Vector2i latticeLimitPoint;
    int mine1, mine2, maxe1, maxe2;

    mine1 = widthCount;
    mine2 = heightCount;
    maxe1 = maxe2 = 0;
    switch(ref->getDefectType())
    {
        case DEFECT_TYPE_DISK:
        case DEFECT_TYPE_PIXEL:
        default:
            for (int i = 0; i < 4; i++)
            {
                latticeLimitPoint = getClosestLatticePoint(limits.at(i));
                if(latticeLimitPoint.x() < mine1)
                    mine1 = latticeLimitPoint.x();
                if(latticeLimitPoint.x() > maxe1)
                    maxe1 = latticeLimitPoint.x();
                if(latticeLimitPoint.y() < mine2)
                    mine2 = latticeLimitPoint.y();
                if(latticeLimitPoint.y() > maxe2)
                    maxe2 = latticeLimitPoint.y();
            }
            break;
    }
    for(int i = mine1; i <= maxe1; i++)
        for(int j = mine2; j <= maxe2; j++)
        defectPosition[i][j].push_back(ref);
    // Now do the repeats
    std::vector<Vector2d> originalPoints, repeatedPoints;
    Vector2d tmpvec;
    if(boundaryConditions == neuron::LATTICE_BOUNDARIES_PERIODIC)
    {
        originalPoints = def.getPoints();
        for(int k = 0; k < 3; k++)
        {
            tmpvec = originalPoints.at(0)-origin;
            repeatedPoints.clear();
//            repeatedPoints.push_back(tmpvec);
            repeatedPoints.push_back(originalPoints.at(0));
            if(k == 0)
            {
                if(tmpvec.x() < widthCount*e1.norm()/2.)
                    repeatedPoints.at(0).x() += widthCount*e1.norm()/2.;
                else
                    repeatedPoints.at(0).x() -= widthCount*e1.norm()/2.;
            }
            if(k == 1)
            {
                if(tmpvec.y() > -heightCount*e2.norm()/2.)
                    repeatedPoints.at(0).y() -= heightCount*e2.norm()/2.;
                else
                    repeatedPoints.at(0).y() += heightCount*e2.norm()/2.;
            }
            if(k == 2)
            {

                if(tmpvec.x() < widthCount*e1.norm()/2.)
                    repeatedPoints.at(0).x() += widthCount*e1.norm()/2.;
                else
                    repeatedPoints.at(0).x() -= widthCount*e1.norm()/2.;
                if(tmpvec.y() > -heightCount*e2.norm()/2.)
                    repeatedPoints.at(0).y() -= heightCount*e2.norm()/2.;
                else
                    repeatedPoints.at(0).y() += heightCount*e2.norm()/2.;
            }
            def.setPoints(repeatedPoints);
            defectList.push_back(def);
            ref = &defectList.back();

            limits = ref->getDefectLimits();
            mine1 = widthCount;
            mine2 = heightCount;
            maxe1 = maxe2 = 0;
            switch(ref->getDefectType())
            {
                case DEFECT_TYPE_DISK:
                case DEFECT_TYPE_PIXEL:
                default:
                    for (int i = 0; i < 4; i++)
                    {
                        latticeLimitPoint = getClosestLatticePoint(limits.at(i));
                        if(latticeLimitPoint.x() < mine1)
                            mine1 = latticeLimitPoint.x();
                        if(latticeLimitPoint.x() > maxe1)
                            maxe1 = latticeLimitPoint.x();
                        if(latticeLimitPoint.y() < mine2)
                            mine2 = latticeLimitPoint.y();
                        if(latticeLimitPoint.y() > maxe2)
                            maxe2 = latticeLimitPoint.y();
                    }
                    break;
            }
            for(int i = mine1; i <= maxe1; i++)
                for(int j = mine2; j <= maxe2; j++)
                    defectPosition[i][j].push_back(ref);
            }
        }
    return true;
}

// We have lots of repetitions
std::list<Defect> Lattice::getDefectsInRange(std::vector<Vector2d> bounds)
{
    std::list<Defect> dlist;

    Vector2i latticeLimitPoint;
    int mine1, mine2, maxe1, maxe2;
    mine1 = widthCount;
    mine2 = heightCount;
    maxe1 = maxe2 = 0;

    for(std::vector<Vector2d>::iterator i= bounds.begin(); i != bounds.end(); i++)
    {
        // Need to do sets of 4 - the boxes
        latticeLimitPoint = getClosestLatticePoint(*i);
        if(latticeLimitPoint.x() < mine1)
            mine1 = latticeLimitPoint.x();
        if(latticeLimitPoint.x() > maxe1)
            maxe1 = latticeLimitPoint.x();
        if(latticeLimitPoint.y() < mine2)
           mine2 = latticeLimitPoint.y();
       if(latticeLimitPoint.y() > maxe2)
           maxe2 = latticeLimitPoint.y();
    }
    switch(boundaryConditions)
    { 
        case neuron::LATTICE_BOUNDARIES_REFLECTIVE:
        default:
            for(int k = mine1; k <= maxe1; k++)
                for(int l = mine2; l <= maxe2; l++)
                    for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                    {
                        dlist.push_back(*(*m));
                    }
            break;
/*        case LATTICE_BOUNDARIES_PERIODIC:
            // Overflows
            if(((maxe1-mine1) > widthCount/2) && ((maxe2-mine2) < heightCount/2))
            {
                for(int k = 0; k <= mine1; k++)
                    for(int l = mine2; l <= maxe2; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                for(int k = maxe1; k < widthCount; k++)
                    for(int l = mine2; l <= maxe2; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
            }
            else if(((maxe1-mine1) < widthCount/2) && ((maxe2-mine2) > heightCount/2))
            {
                for(int k = mine1; k <= maxe1; k++)
                {
                    for(int l = 0; l <= mine2; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                    for(int l = maxe2; l < heightCount; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                }
            }
            else if(((maxe1-mine1) > widthCount/2) && ((maxe2-mine2) > heightCount/2))
            {
                for(int k = 0; k <= mine1; k++)
                {
                    for(int l = 0; l <= mine2; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                    for(int l = maxe2; l < heightCount; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                }
                for(int k = maxe1; k < widthCount; k++)
                {
                    for(int l = 0; l <= mine2; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                    for(int l = maxe2; l < heightCount; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
                }
            }
            else
                for(int k = mine1; k <= maxe1; k++)
                    for(int l = mine2; l <= maxe2; l++)
                        for(std::vector<Defect*>::iterator m = defectPosition[k][l].begin(); m != defectPosition[k][l].end(); m++)
                        dlist.push_back(*(*m));
            break;*/
    }


        //std::cout << "1.9\n";
//    }
    return dlist;
}

// Input in real cartesians - output in lattice basis 
Vector2i Lattice::getClosestLatticePoint(Vector2d point)
{
    Vector2d tmpvec = point-origin;
    // Now we have the point in lattice space - go to the new coordinate basis - most of the time is just a rescaling
    double determinant = e1.x()*e2.y()-e2.x()*e1.y(); 
    Vector2d newbasis = Vector2d(e2.y()*tmpvec.x()+e1.y()*tmpvec.y(), e2.x()*tmpvec.x()+e1.x()*tmpvec.y())/determinant;

    Vector2i closest(floor(newbasis.x()), floor(newbasis.y())); // Lattice

    switch(boundaryConditions)
    { 
        case neuron::LATTICE_BOUNDARIES_REFLECTIVE:
        default:        
        // Check ranges so it falls inside the lattice
            if(closest.x() < 0)
                closest = Vector2i(0, closest.y());
            if(closest.x() >= widthCount)
                closest = Vector2i(widthCount-1, closest.y());
            if(closest.y() < 0)
                closest = Vector2i(closest.x(), 0);
            if(closest.y() >= heightCount)
                closest = Vector2i(closest.x(), heightCount-1);
            break;
/*        case LATTICE_BOUNDARIES_PERIODIC:
            if(closest.x() < 0)
                closest = Vector2i(widthCount+closest.x(), closest.y());
            if(closest.x() >= widthCount)
                closest = Vector2i(closest.x()-widthCount, closest.y());
            if(closest.y() < 0)
                closest = Vector2i(closest.x(), heightCount+closest.y());
            if(closest.y() >= heightCount)
                closest = Vector2i(closest.x(), closest.y()-heightCount);
            break;*/
    }
    return closest;

}
  
bool Lattice::firstBoundaryOverflow(Vector2d point)
{
    Vector2d tmpvec = point-origin;
    // Now we have the point in lattice space - go to the new coordinate basis - most of the time is just a rescaling
    double determinant = e1.x()*e2.y()-e2.x()*e1.y(); 
    Vector2d newbasis = Vector2d(e2.y()*tmpvec.x()+e1.y()*tmpvec.y(), e2.x()*tmpvec.x()+e1.x()*tmpvec.y())/determinant;

    Vector2i closest(floor(newbasis.x()), floor(newbasis.y())); // Lattice

    if((closest.x() < 0) || closest.x() >= widthCount)
        return true;
    else
        return false;
}

bool Lattice::secondBoundaryOverflow(Vector2d point)
{
    Vector2d tmpvec = point-origin;
    // Now we have the point in lattice space - go to the new coordinate basis - most of the time is just a rescaling
    double determinant = e1.x()*e2.y()-e2.x()*e1.y(); 
    Vector2d newbasis = Vector2d(e2.y()*tmpvec.x()+e1.y()*tmpvec.y(), e2.x()*tmpvec.x()+e1.x()*tmpvec.y())/determinant;

    Vector2i closest(floor(newbasis.x()), floor(newbasis.y())); // Lattice

    if((closest.y() < 0) || closest.y() >= heightCount)
        return true;
    else
        return false;
}


Vector2d Lattice::fromAbsoluteToPeriodic(Vector2d point)
{
    Vector2d tmpvec = point-origin;
    Vector2d periodic;
    
    periodic.x() = fmod(tmpvec.x(), widthCount*e1.norm());
    periodic.y() = fmod(tmpvec.y(), heightCount*e2.norm());
    std::cout << point.x() << " " << (periodic+origin).x() << " " << point.y() << " " << (periodic+origin).y() << 
        "\n";

    return periodic+origin;
}
