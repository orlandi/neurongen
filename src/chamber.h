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

#ifndef _CHAMBER_H_
#define _CHAMBER_H_

#include <vector>
#include <Eigen/Core>
#include "neuron.h"
#include "neuronnamespace.h"

// import most common Eigen types 
//USING_PART_OF_NAMESPACE_EIGEN

using namespace Eigen;

class Lattice;
class Pattern;
class Defect;

class Chamber
{
    public:
        Chamber();
        Chamber(neuron::chamberParameters p);
        void draw();
        inline void setCultureParameters(neuron::cultureParameters cparam)
            {cultureParam = cparam;}
        inline void setNeuronParameters(neuron::somaParameters sparam, neuron::dtreeParameters dparam, neuron::axonParameters aparam)
            {somaParam = sparam;
             dtreeParam = dparam;
             axonParam = aparam;}
        bool assignLattice(int boundaries, Vector2d orig, double unitWidth, double unitHeight, double wid, double hei);
        bool assignLattice();
        bool assignPatternDefects();
        bool assignDensityMap();
        bool insertNeurons(int num = 0);
        bool growAxons();
        bool growDendrites();
        bool growConnections();
        std::vector<Neuron*> addConnections(Neuron& origin);
        std::vector<Vector2d> growSingleAxon(Vector2d origin);

        void setActiveZone(Vector2d center, double radius);
        inline void setRNG(gsl_rng* rngp)
            {rng = rngp;}
        inline neuron::dtreeParameters getDtreeParameters()
            {return dtreeParam;}
        Vector2d getEmptySpot();
        Defect getEmptySpot(Defect def);
        bool checkIntersections(Defect def);
        std::vector<Neuron> neuron;

    private:
        void init();
        void postInit();
        void normalizeUnits();

        bool displayList, activeZone, densityMap;
        Vector2d activeZoneCenter;
        double activeZoneRadius;

        neuron::chamberParameters param;
        neuron::cultureParameters cultureParam;
        neuron::somaParameters somaParam;
        neuron::dtreeParameters dtreeParam;
        neuron::axonParameters axonParam;

        int totalNeurons;
 
        Pattern* pattern;
        Lattice* lattice;
        gsl_rng* rng;
        std::vector<double> densityMapX, densityMapY, densityMapP;
        double densityMapPointWidth, densityMapPointHeight;
        gsl_ran_discrete_t* densityMapLookupTable;
};

#endif
    // _CHAMBER_H_

