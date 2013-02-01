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

#include <fstream>
#include <vector>
#include <list>
#include <set>
#include "lattice.h"
#include "neuron.h"
#include "pattern.h"
#include "defect.h"
#include "chamber.h"

Chamber::Chamber()
{
    init();
    postInit();
}

Chamber::Chamber(neuron::chamberParameters p)
{
    init();
    param = p;
    postInit();
}

// Variable preinitialization
void Chamber::init()
{
    pattern = NULL;
    lattice = NULL;
    displayList = false;
    activeZone = false;
    densityMap = false;
    totalNeurons = 0;

    param = neuron::DEFAULT_CHAMBER_PARAMETERS; 
}

bool Chamber::assignLattice(int boundaries, Vector2d orig, double unitWidth, double unitHeight, double wid, double hei)
{
    if(!lattice)
        lattice = new Lattice(boundaries, orig, unitWidth, unitHeight, wid, hei);

    return true;
}

bool Chamber::assignLattice()
{
    if(!lattice && pattern)
        lattice = new Lattice(neuron::LATTICE_BOUNDARIES_PERIODIC, pattern->getOrigin(), somaParam.radius*3., somaParam.radius*3.,
//        lattice = new Lattice(LATTICE_BOUNDARIES_REFLECTIVE, pattern->getOrigin(), somaParam.radius*3., somaParam.radius*3.,
                (pattern->getSize()).x(), (pattern->getSize()).y());
//    std::cout << "Lattice size: " << (pattern->getSize()).x() << " " << (pattern->getSize()).y() << "\n";
    return true;
}

bool Chamber::assignPatternDefects()
{
    if(!lattice || !pattern)
        return false;
    Vector2i patternSize = pattern->getSizeCount();
    Vector2d pixelSize = pattern->getUnitSize();
    std::vector<double> pixelsize;
    pixelsize.push_back(pixelSize.x());
    pixelsize.push_back(pixelSize.y());

    Vector2d pos;
    Defect pdefect;
    for (int x = 0; x < patternSize.x(); x++)
        for (int y = 0; y < patternSize.y(); y++)
        {
            if(pattern->checkPattern(x, y))
            {
                pos = pattern->getPosition(x, y);
                pdefect = Defect(DEFECT_TYPE_PIXEL, DEFECT_CLASS_PATTERN, 0, pixelsize, std::vector<Vector2d>(1, pos));
//                std::cout << x << " " << y << " " << pos.x() << " " << pos.y() << "\n";
                lattice->addDefect(pdefect);
            }
        }
    return true; 
}

// Variable postinitialization
void Chamber::postInit()
{
    normalizeUnits();

    if(param.type == neuron::CH_TYPE_CUSTOM)
    {
        if(param.pattern == true)
        {
            pattern = new Pattern(param.width, param.height);
            pattern->loadPatternFromFile(param.patternFile);
        }
    }
}

bool Chamber::insertNeurons(int num)
{
    int newNeurons;
    if(num <= 0)
        newNeurons = cultureParam.neuronNumber;
    else
        newNeurons = num;
    
    totalNeurons += newNeurons;

    std::vector<Neuron>::iterator it = neuron.end();
    int tnumber = it-neuron.begin();
//    neuron.insert(it, newNeurons, Neuron(somaParam, this, rng, neuronQuadric));
    neuron.insert(it, newNeurons, Neuron(somaParam, axonParam, dtreeParam, this, rng));

    Vector2d newPos;
    std::vector<Vector2d > poi;
    poi.push_back(newPos);
    Defect defneuron;
    std::vector<double> nsize;
    nsize.push_back(somaParam.radius);
    int j = 0, idx;

    std::cout << "Placing Neuron... 0\n";
    for(std::vector<Neuron>::iterator i=(neuron.begin()+tnumber); i != neuron.end(); i++)
    {
//        nsize.at(0) = i->getSomaRadius()*gsl_ran_flat(rng, 0.75, 1.25);
//        i->setSomaRadius(nsize.at(0));
        idx = i-neuron.begin();
        defneuron = Defect(DEFECT_TYPE_DISK, DEFECT_CLASS_SOMA, 
                           neuron::COL_PATTERN & neuron::COL_BOUNDARIES & neuron::COL_SOMAS, nsize, poi, idx);
        defneuron = getEmptySpot(defneuron);
        lattice->addDefect(defneuron);
        i->setPosition(defneuron.getPoints());

    //        std::cout << j << "\n";
        if(fmod(j+1, 1000.0) == 0.0)
            std::cout << "Placing Neuron... " << j+1 << "\n";
        j++;
    }

    return true;
}

bool Chamber::growAxons()
{
    int j = 0;
    for(std::vector<Neuron>::iterator i=neuron.begin(); i != neuron.end(); i++)
    {
        i->growAxon();

        if(fmod(j+1, 1000.0) == 0.0)
            std::cout << "Growing Axon... " << j+1 << "\n";
        j++;
    }
    return true;
}

bool Chamber::growDendrites()
{
    int idx;
    Defect dend;
    for(std::vector<Neuron>::iterator i=neuron.begin(); i != neuron.end(); i++)
    {
        idx = i-neuron.begin();
        dend = i->growDendrites();
        dend.setIndex(idx);
        i->setIndex(idx);
        lattice->addDefect(dend);
    }
    return true;
}

std::vector<Neuron*> Chamber::addConnections(Neuron& origin)
{
    // Get all defects around the chain
    Defect axon = origin.getAxon();
    std::vector<Vector2d> points, bounds;
    points = axon.getPoints();
    std::list<Defect> dlist, tmplist;
    std::set<int> neuronIndex;
    Defect dendrite;
    std::vector<Neuron*> outputConnections;
//    std::cout << origin.getPosition().x() << " ";
/*    for(std::vector<Vector2d>::iterator i = points.begin(); i < points.end()-1; i++)
    {
        bounds.clear();
        bounds.push_back(*i);
        bounds.push_back(*(i+1));
        tmplist = lattice->getDefectsInRange(bounds);
        // Warning in here
        dlist.insert(dlist.end(), tmplist.begin(), tmplist.end());
    }

    // Extract unique dendrites from the defect list
    for(std::list<Defect>::iterator i = dlist.begin(); i != dlist.end(); i++)
    {
        neuronIndex.insert(i->getIndex());
    }*/

    for(std::vector<Vector2d>::iterator i = points.begin(); i < points.end()-1; i++)
    {
        bounds.clear();
        bounds.push_back(*i);
        bounds.push_back(*(i+1));
        tmplist = lattice->getDefectsInRange(bounds);
        // Extract unique dendrites from the defect list
        for(std::list<Defect>::iterator j = tmplist.begin(); j != tmplist.end(); j++)
            if(j->getClassType() == DEFECT_CLASS_DTREE)
                neuronIndex.insert(j->getIndex());
    }

    // Go trhough all the dendrites and check for intersections
    for(std::set<int>::iterator i = neuronIndex.begin(); i != neuronIndex.end(); i++)
    {
        if(*i == origin.getIndex())
            continue;
        dendrite = neuron.at(*i).getDendrites();
        if(axon.intersect(dendrite, lattice->getBoundaryConditions(), lattice->getWidth(), lattice->getHeight()))
        {
            outputConnections.push_back(&(neuron.at(*i)));
        }
//            std::cout << *i << " ";
    }
//    std::cout << " : " << outputConnections.size() << "\n";
    origin.setOutputConnections(outputConnections);
    return outputConnections;
}

bool Chamber::growConnections()
{
    std::vector<Neuron*> outputConnections;
    std::vector<std::vector<Neuron*> > inputConnections;
    inputConnections.insert(inputConnections.end(), neuron.size(), std::vector<Neuron* >());

    int j = 0;
    for(std::vector<Neuron>::iterator i=neuron.begin(); i != neuron.end(); i++)
    {
        outputConnections = addConnections(*i);

        for(std::vector<Neuron*>::iterator k=outputConnections.begin(); k != outputConnections.end(); k++)
        {
           
//            inputConnections.push_back(std::vector<Neuron* >());
//            (inputConnections.at(i-neuron.begin())).push_back(*k);
            inputConnections.at((*k)->getIndex()).push_back(&(*i));
        }
        if(fmod(j+1, 1000.0) == 0.0)
            std::cout << "Creating Output Connection... " << j+1 << "\n";
        j++;
    }

    std::cout << "Assigning Input Connections... " << "\n";
    for(std::vector<Neuron>::iterator i=neuron.begin(); i != neuron.end(); i++)
        i->setInputConnections(inputConnections.at(i-neuron.begin()));
    return true;
}

bool Chamber::assignDensityMap(std::string fileName, double width, double height)
{
    std::ifstream inputFile;
    std::string line, tmpString, lineString;
    std::stringstream tmpStr, lineStream;
    double tmpX, tmpY, tmpP;
    size_t events;
    double* eventProbabilities;

    inputFile.open(fileName.c_str(), std::ifstream::in);
    if (!inputFile.is_open())
    {
        std::cout << "There was an error opening the file " << fileName << "\n";
        return false;
    }

    std::cout << "Loading density map...\n";
    densityMapX.clear();
    densityMapY.clear();
    densityMapP.clear();
    while(std::getline(inputFile, line))
    {
        if(line.at(0) == '%')
            continue;
/*        tmpString = "";
        tmpStr.clear();
        tmpStr.str(line);

        std::getline(tmpStr, tmpString,' ');
        lineStream.clear();
        lineStream.str(tmpString);
        lineStream >> tmpX;
        
        std::getline(tmpStr, tmpString,' ');
        lineStream.clear();
        lineStream.str(tmpString);
        lineStream >> tmpY;
        
        std::getline(tmpStr, tmpString,' ');
        lineStream.clear();
        lineStream.str(tmpString);
        lineStream >> tmpP;*/
        lineStream.clear();
        lineStream.str(line);
        lineStream >> tmpX >> tmpY >> tmpP;
        
        densityMapX.push_back(tmpX);
        densityMapY.push_back(tmpY);
        densityMapP.push_back(tmpP);
//        std::cout << tmpX << " " << tmpY << " " << tmpP << "\n";
    }
    inputFile.close();
    densityMapPointWidth = width;
    densityMapPointHeight = height;
    events = densityMapX.size();
    eventProbabilities = new double[events];
    for(int i = 0; i < events; i++)
        eventProbabilities[i] = densityMapP.at(i);
    densityMapLookupTable = gsl_ran_discrete_preproc(events, eventProbabilities);
    densityMap = true;
    param.type = neuron::CH_TYPE_CUSTOM_WITH_DENSITY_MAP;

    return true;
}

Defect Chamber::getEmptySpot(Defect def)
{
    double tmpX, tmpY;
    std::vector<Vector2d> points;
    bool valid = false;
    int retries = 0;
    int maxretries = 1000;
    size_t tmpIndex;
    while(!valid)
    {
        retries++;
        valid = true;
        switch(param.type)
        {
            case neuron::CH_TYPE_CIRCULAR:
                break;
            case neuron::CH_TYPE_RECTANGULAR:
            case neuron::CH_TYPE_CUSTOM_WITH_DENSITY_MAP:
                tmpIndex = gsl_ran_discrete(rng, densityMapLookupTable);
                tmpX = densityMapX.at(tmpIndex);
                tmpY = densityMapY.at(tmpIndex);
                tmpX += gsl_ran_flat(rng, 0., 1.)*densityMapPointWidth;
                tmpY -= gsl_ran_flat(rng, 0., 1.)*densityMapPointHeight;
//                std::cout << tmpX << " " << densityMapX.at(tmpIndex) << "\n";
                points.clear();
                points.push_back(Vector2d(tmpX, tmpY));
                break;
            case neuron::CH_TYPE_CUSTOM:
            default:
                tmpX = gsl_ran_flat(rng, -0.5,0.5)*param.width;
                tmpY = gsl_ran_flat(rng, -0.5,0.5)*param.height;
                points.clear();
                points.push_back(Vector2d(tmpX, tmpY));
                break;
        }

//            std::cout << tmpX << " " << tmpY << "\n";
        def.setPoints(points);
  //          std::cout << "empty1.6\n";
        std::list<Defect> dlist = lattice->getDefectsInRange(def.getDefectLimits());
//        if(dlist.size() > 0)
//            std::cout << "Defects: " << dlist.size() << "\n";
        for(std::list<Defect>::iterator i = dlist.begin(); i != dlist.end(); i++)
        {
            if(def.intersect(*i) && (retries < maxretries))
            {
                valid = false;
                break;
            }
            else if(retries >= maxretries)
            {
                std::cout << "Retry limit reached\n";
                break;
            }
        }
    }
    return def;
}

bool Chamber::checkIntersections(Defect def)
{
    std::list<Defect> dlist = lattice->getDefectsInRange(def.getDefectLimits());
    for(std::list<Defect>::iterator i = dlist.begin(); i != dlist.end(); i++)
        if(def.intersect(*i))
            return true;
    
    return false;
}

Vector2d Chamber::getEmptySpot()
{
    double tmpX, tmpY;
    tmpX = gsl_ran_flat(rng, -0.5,0.5)*(pattern->getSize()).x();
    tmpY = gsl_ran_flat(rng, -0.5,0.5)*(pattern->getSize()).y();

    return Vector2d(tmpX, tmpY);
}

// Default working unit is milimeters
void Chamber::normalizeUnits()
{
    double multiplier; 
    switch(param.units)
    {
        case neuron::UNITS_CENTIMETERS:
            multiplier = 10;
            break;

        case neuron::UNITS_MILIMETERS:
            multiplier = 1;
            break;

        case neuron::UNITS_MICROMETERS:
            multiplier = 0.001;
            break;
        case neuron::UNITS_NANOMETERS:
            multiplier = 0.000001;
            break;
        default:
            multiplier = 1;
            break;
    }
    param.width *= multiplier;
    param.height *= multiplier;
    param.radius *= multiplier;
    param.units = neuron::UNITS_MILIMETERS;
}

void Chamber::setActiveZone(Vector2d center, double radius)
{
    activeZone = true;
    activeZoneCenter = center;
    activeZoneRadius = radius;
}

