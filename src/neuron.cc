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

#include "neuron.h"
#include "chamber.h"
#include "defect.h"

Neuron::Neuron()
{
    somaParams = neuron::DEFAULT_SOMA_PARAMETERS;
    axonParams = neuron::DEFAULT_AXON_PARAMETERS;
    dtreeParams = neuron::DEFAULT_DTREE_PARAMETERS;
    somaRadius = somaParams.radius;
    rng = NULL;
    init();
}

Neuron::Neuron(neuron::somaParameters param, neuron::axonParameters aparam, neuron::dtreeParameters dparam, 
               Chamber* cham, gsl_rng* parentrng)
{
    somaParams = param;
    axonParams = aparam;
    dtreeParams = dparam;
    chamber = cham;
    rng = parentrng;
    switch(somaParams.shape)
    {
        case neuron::SOMA_SHAPE_CIRCULAR:
        default:
            somaRadius = somaParams.radius;
            break;
    }
    init();
}

void Neuron::init()
{
    if(rng)
    {
        cR = gsl_ran_flat(rng, 0., 1.);
        cB = gsl_ran_flat(rng, 0., 1.);
        cG = gsl_ran_flat(rng, 0., 1.);
    }
    else
    {
        cB = cG = 0.;
        cR = 1.;
    }
}

void Neuron::growAxon()
{
    int trial, retry;
    bool success;
    double angle;
    Vector2d newSegment, endPoint;
    std::vector<Vector2d> newSegmentDefectPoints;
    Defect newSegmentDefect;

    switch(axonParams.lengthDistribution)
    {
        case neuron::DISTRIBUTION_RAYLEIGH:
        default:
            //axonLength = gsl_ran_rayleigh(rng, axonParams.meanLength);
            axonLength = gsl_ran_rayleigh(rng, axonParams.stdLength);
            break;
    }

    switch(axonParams.type)
    {
        case neuron::AXON_TYPE_SEGMENTED:
        default:
            switch(axonParams.segmentType)
            {
                case neuron::AXON_STYPE_FIXEDNUMBER:
                    axonParams.segmentLength = axonLength/axonParams.segmentCount;
                    break;
                case neuron::AXON_STYPE_FIXEDLENGTH:
                default:
                    axonParams.segmentCount = ceil(axonLength/axonParams.segmentLength);
                    if(axonParams.segmentCount <= 1)
                    {
                        axonLength += axonParams.segmentLength;
                        axonParams.segmentCount++;
                    }
                    break;
            }
            break;
    }
    // Real growth starts here
    for(int i = 0; i < axonParams.segmentCount; i++)
    {
        trial = 1;
        retry = 0;
        success = false;
        while(!success)
        {
            // The first segment is trivial
            if(i == 0)
            {
                angle = gsl_ran_flat(rng, 0., 2.*M_PI);
                newSegment = Vector2d(cos(angle), sin(angle))*axonParams.segmentLength;
            }
            else
            {
                switch(axonParams.segmentAngleDistribution)
                {
                    case neuron::DISTRIBUTION_UNIFORM:
                        angle = axonParams.meanSegmentAngle + gsl_ran_flat(rng, -axonParams.stdSegmentAngle, 
                                axonParams.stdSegmentAngle)*trial;
                        break;
                    case neuron::DISTRIBUTION_GAUSSIAN:
                    default:
                        angle = axonParams.meanSegmentAngle + gsl_ran_gaussian_ziggurat(rng, axonParams.stdSegmentAngle*trial);
                        break;
                }
                // Rotate the new angle respect the last vector
                if(i > 1)
                    newSegment = Eigen::Rotation2D<double>(angle)*
                                 (axonSegments.at(i-1)-axonSegments.at(i-2));
                else
                    newSegment = Eigen::Rotation2D<double>(angle)*
                                 (axonSegments.at(i-1)-position);

//            std::cout << "Angle: " << angle << "\n";
            }
            // Last segment always has a special length
            if(i == axonParams.segmentCount-1)
            {
                newSegment = newSegment.normalized()*fmod(axonLength, axonParams.segmentLength); 
            }
            // Get the final point
/*            endPoint = Vector2d(position.x(), position.y());
            for(int j = 0; j < i; j++)
            {
                endPoint += axonSegments.at(j);
            }
            endPoint += newSegment;*/
            if(i > 0)
                endPoint = axonSegments.at(i-1)+newSegment;
            else
                endPoint = position+newSegment;

            // Now that we have the new segment check if it collides with anything
            newSegmentDefectPoints.clear();
            newSegmentDefectPoints.push_back(endPoint-newSegment);
            newSegmentDefectPoints.push_back(endPoint);
            newSegmentDefect =
            Defect(DEFECT_TYPE_SEGMENT, DEFECT_CLASS_AXON, 0, std::vector<double>(1, axonParams.segmentLength),
                    newSegmentDefectPoints);
            if(chamber->checkIntersections(newSegmentDefect) && (axonParams.stdSegmentAngle*trial < 
                                                                 axonParams.maxStdSegmentAngle))
            {
                retry++;
                if(retry >= axonParams.maxRetries)
                {
                    retry = 0;
                    trial++;
                }
                success = false;
            }
            else if(axonParams.stdSegmentAngle*trial > axonParams.maxStdSegmentAngle)
            {
                success = true;
                std::cout << "Axon limit reached\n";
            }
            else
                success = true;
        }
//        axonSegments.push_back(newSegment);
        axonSegments.push_back(endPoint);
    }
}

double Neuron::getAxonEndToEndDistance()
{
    Vector2d curPos = axonSegments.back()-position;
    return curPos.norm();
}

std::vector<Neuron*> Neuron::getUndirectedConnections()
{
    std::vector<Neuron*> undirected = inputConnections;
    undirected.insert(undirected.end(), outputConnections.begin(), outputConnections.end());
    return undirected;
}

Defect Neuron::growDendrites()
{
    double multiplier;
    switch(dtreeParams.shape)
    {
        case neuron::DTREE_SHAPE_CIRCULAR:
        default:
            switch(dtreeParams.sizeDistribution)
            {
                case neuron::DISTRIBUTION_GAUSSIAN:
                    dtreeRadius = dtreeParams.meanRadius+gsl_ran_gaussian_ziggurat(rng, dtreeParams.stdRadius);
                    break;

                case neuron::DISTRIBUTION_CUX:
                    if(gsl_ran_flat(rng, 0., 1.) <= dtreeParams.CUXfraction)
                    {
                        multiplier = dtreeParams.CUXmultiplier;
                        CUXactive = true;
                    }
                    else
                    {
                        multiplier = 1.;
                        CUXactive = false;
                    }
                    dtreeRadius = dtreeParams.meanRadius*multiplier+gsl_ran_gaussian_ziggurat(rng, dtreeParams.stdRadius);
                    break;

                case neuron::DISTRIBUTION_RAYLEIGH:
                default:
                    dtreeRadius = gsl_ran_rayleigh(rng, dtreeParams.meanRadius);
                    break;
            }
            break;
    }
    if(dtreeRadius < somaRadius*2.)
        dtreeRadius = somaRadius*2.;

    return Defect(DEFECT_TYPE_DISK, DEFECT_CLASS_DTREE,
                          0x00, std::vector<double>(1, dtreeRadius), std::vector<Vector2d>(1, position));
}

Defect Neuron::getDendrites()
{
    return Defect(DEFECT_TYPE_DISK, DEFECT_CLASS_DTREE,
                          0x00, std::vector<double>(1, dtreeRadius), std::vector<Vector2d>(1, position));
}

Defect Neuron::getAxon()
{
    return Defect(DEFECT_TYPE_CHAIN, DEFECT_CLASS_AXON, 0x00, std::vector<double>(1, axonLength), axonSegments);
}

void Neuron::printPovRayStructure()
{
    Vector2d curpos, nextpos;

    std::cout << "union {\nsphere{\n < " << position.x() << ", " << position.y() << ", 0 >, "
              << somaRadius << "\n}\n";
    curpos = position;
    for(std::vector<Vector2d>::iterator i = axonSegments.begin(); i != axonSegments.end(); i++)
    {
        nextpos = curpos+*i;
        std::cout << "cylinder { <" << curpos.x() << ", " << curpos.y() << ", 0>, <"
                  << nextpos.x() << ", " << nextpos.y() << ", 0>, " << axonParams.width/2.
                  << " }\n";
        curpos = nextpos;
    }
    std::cout << "\n";
}



