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

#include "defect.h"

Defect::Defect()
{

}

Defect::Defect(defectType typ, defectClassIndex clas, int overl, std::vector<double> siz, std::vector<Vector2d > poi, int idx)
{
    type = typ;
    classType = clas;
    overlap = overl;
    sizes = siz;
    points = poi;
    index = idx;
}

std::vector<Vector2d> Defect::getDefectLimits()
{
    Vector2d currPoint;
    std::vector<Vector2d> limits;
//    limits.reserve(4);
    switch(type)
    {
        case DEFECT_TYPE_DISK:
        default:
            currPoint = points.at(0);
            limits.push_back(currPoint-Vector2d(-1.,0.)*sizes.at(0));
            limits.push_back(currPoint-Vector2d(1.,0.)*sizes.at(0));
            limits.push_back(currPoint-Vector2d(0.,1.)*sizes.at(0));
            limits.push_back(currPoint-Vector2d(0.,-1.)*sizes.at(0));
            break;
        case DEFECT_TYPE_PIXEL:
            currPoint = points.at(0);
            limits.push_back(currPoint);
            limits.push_back(currPoint+Vector2d(1.,0.)*sizes.at(0));
            limits.push_back(currPoint+Vector2d(1.,0.)*sizes.at(0)-Vector2d(0.,1.)*sizes.at(1));
            limits.push_back(currPoint-Vector2d(0.,1.)*sizes.at(1));
            break;
        case DEFECT_TYPE_SEGMENT:
            limits.push_back(points.at(0));
            limits.push_back(points.at(1));
            break;
    }
    return limits;
}

bool Defect::intersect(Defect newDefect, int boundaries, double w, double h)
{
    std::vector<double> newSizes = newDefect.getSizes();
    std::vector<Vector2d> newPoints = newDefect.getPoints();
    std::vector<Vector2d> newLimits;
    Vector2d lline, rline, seg1, seg2, p1, p2, p3, p4, tmpvec;
    double dist, det;
    int k, k1, k2;
//    double spacing;
    switch(type)
    {
        case DEFECT_TYPE_DISK:
        default:
            if(newDefect.getDefectType() == DEFECT_TYPE_DISK)
            {
                if((newPoints.at(0)-points.at(0)).norm() <= fabs(newSizes.at(0)+sizes.at(0)))
                    return true;
            }
            if(newDefect.getDefectType() == DEFECT_TYPE_PIXEL)
            {
                newLimits = newDefect.getDefectLimits();
                k = 0;
                for(int i = 0; i < 4; i++)
                {
                    // First check if it is inside
                    lline = newLimits.at(i);
                    if(i == 3)
                        rline = newLimits.at(0);
                    else
                        rline = newLimits.at(i+1);
                    seg1 = rline-lline;
                    seg2 = points.at(0)-lline;
                    if(seg1.x()*seg2.y()-seg1.y()*seg2.x() <= 0.)
                        k++;
                    if(k == 4)
                        return true;
                
                    // Now check if it intersects with any line
                    dist = ((points.at(0).x()-lline.x())*(rline.x()-lline.x())
                           +(points.at(0).y()-lline.y())*(rline.y()-lline.y()))
                           /((lline-rline).norm()*(lline-rline).norm());
                    if((dist >= 0.) && (dist <= 1.))
                    {
                        det = sqrt((lline.x()-points.at(0).x()+dist*(rline.x()-lline.x()))*
                                   (lline.x()-points.at(0).x()+dist*(rline.x()-lline.x()))+
                                   (lline.y()-points.at(0).y()+dist*(rline.y()-lline.y()))*
                                   (lline.y()-points.at(0).y()+dist*(rline.y()-lline.y())));
                        if(det <= sizes.at(0))
                            return true;
                    }
                    if((points.at(0)-lline).norm() < sizes.at(0))
                    {
                        return true;
                    }
                    if((points.at(0)-rline).norm() < sizes.at(0))
                    {
                        return true;
                    }
                }
            }
            break;

        case DEFECT_TYPE_SEGMENT:
            if(newDefect.getDefectType() == DEFECT_TYPE_PIXEL)
            {
                newLimits = newDefect.getDefectLimits();
                k1 = k2 = 0;
                p1 = points.at(0);
                p2 = points.at(1);
                // First check if any point in the segment is inside the pixel
                for(int i = 0; i < 4; i++)
                {
                    for(int j = 0; j < 2; j++)
                    {
                        lline = newLimits.at(i);
                        if(i == 3)
                            rline = newLimits.at(0);
                        else
                            rline = newLimits.at(i+1);
                        seg1 = rline-lline;
                        seg2 = points.at(j)-lline;
                        if(seg1.x()*seg2.y()-seg1.y()*seg2.x() <= 0.)
                        {
                            if(j == 0)
                                k1++;
                            else
                                k2++;
                        }
                        if((k1 == 4) || (k2 == 4))
                            return true;
                    }
                    p3 = lline;
                    p4 = rline;
                    // Now check if it intersects with any line
                    det = (p4.y()-p3.y())*(p2.x()-p1.x())-(p4.x()-p3.x())*(p2.y()-p1.y());
                    if(det == 0)
                        continue;
/*                    seg1.x() = (p1.x()*p2.y()-p2.x()*p1.y())*(p3.x()-p4.x())-(p3.x()*p4.y()-p4.x()*p3.y())*(p1.x()-p2.x());
                    seg1.x() /= det;
                    seg1.y() = (p1.x()*p2.y()-p2.x()*p1.y())*(p3.y()-p4.y())-(p3.x()*p4.y()-p4.x()*p3.y())*(p1.y()-p2.y());
                    seg1.y() /= det;*/

                    // Old
                    dist = (p4.x()-p3.x())*(p1.y()-p3.y())-(p4.y()-p3.y())*(p1.x()-p3.x());
                    dist /= det;
                    if((dist >= 0.) && (dist <= 1.))
                    {
                        dist = (p2.x()-p1.x())*(p1.y()-p3.y())-(p2.y()-p1.y())*(p1.x()-p3.x());
                        dist /= det;
                        if((dist >= 0.) && (dist <= 1.))
                            return true;
                    }
                }
            }
            break;
        case DEFECT_TYPE_CHAIN:
            if(newDefect.getDefectType() == DEFECT_TYPE_DISK)
            {
                p3 = newPoints.at(0);
                // First check if any point on the chain is inside the disk 
                for(std::vector<Vector2d>::iterator i = points.begin(); i != points.end(); i++)
                {
                    if((*i-p3).norm() <= newSizes.at(0))
                        return true;
                if(boundaries == neuron::LATTICE_BOUNDARIES_PERIODIC)
                {
                    for(int k = 0; k < 3; k++)
                    {
                        tmpvec = p3;
                        if(k == 0)
                        {
                            if(p3.x() < 0.)
                                tmpvec.x() += w/2.;
                            else
                                tmpvec.x() -= w/2.;
                        }
                        if(k == 1)
                        {
                            if(p3.y() < 0.)
                                tmpvec.y() += h/2.;
                            else
                                tmpvec.y() -= h/2.;
                        }
                        if(k == 2)
                        {
                            if(p3.x() < 0.)
                                tmpvec.x() += w/2.;
                            else
                                tmpvec.x() -= w/2.;
                            if(p3.y() < 0.)
                                tmpvec.y() += h/2.;
                            else
                                tmpvec.y() -= h/2.;
                        }
                        if((*i-tmpvec).norm() <= newSizes.at(0))
                        {
                            return true;
                        }
                    }
                }
                }
                // If not, then check if the minimum distance from a segment to the center is smaller than the radius
/*                for(std::vector<Vector2d>::iterator i = points.begin(); i < points.end()-1; i++)
                {
                    lline = *i;
                    rline = *(i+1); 
                    dist = ((p3.x()-lline.x())*(rline.x()-lline.x())
                           +(p3.y()-lline.y())*(rline.y()-lline.y()))
                           /(((lline-rline).norm())*((lline-rline).norm()));
                    det = sqrt((lline.x()-p3.x()+dist*(rline.x()-lline.x()))*
                                (lline.x()-p3.x()+dist*(rline.x()-lline.x()))+
                                (lline.y()-p3.y()+dist*(rline.y()-lline.y()))*
                                (lline.y()-p3.y()+dist*(rline.y()-lline.y())));
                    if(det < newSizes.at(0))
                        return true;
                }*/
            }

            break;
    }
    return false;
}


