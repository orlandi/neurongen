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

#ifndef _NEURON_H_
#define _NEURON_H_

#include <Eigen/Geometry>
#include <Eigen/Core>
#include <vector>
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"
//#include <SFML/Window.hpp>
#include "neuronnamespace.h"

//USING_PART_OF_NAMESPACE_EIGEN

using namespace Eigen;

class Chamber;
class Defect;

class Neuron
{
	public:
        Neuron();
        Neuron(neuron::somaParameters param, neuron::axonParameters aparam, neuron::dtreeParameters dparam,
               Chamber* cham, gsl_rng* parentrng = NULL);
        void init();
        inline void setPosition(Vector2d pos)
            {position = pos;}
        inline Vector2d getPosition()
            {return position;}
        inline void setAxonParameters(neuron::axonParameters param)
            {axonParams = param;}
        inline void setPosition(std::vector<Vector2d> pos)
            {position = pos.at(0);}
        inline void setSomaRadius(double rad)
            {somaRadius = rad;}
        inline double getSomaRadius()
            {return somaRadius;}
        inline void setOutputConnections(std::vector<Neuron*> cons)
            {outputConnections = cons;}
        inline void setInputConnections(std::vector<Neuron*> cons)
            {inputConnections = cons;}
        inline void setColor(double red, double green, double blue)
            {cR = red; cG = green; cB = blue;}

        std::vector<Neuron*> getUndirectedConnections();
        inline std::vector<Neuron*> getOutputConnections()
            {return outputConnections;}
        inline std::vector<Neuron*> getInputConnections()
            {return inputConnections;}
        void growAxon();
        inline void setIndex(int idx)
            {index = idx;}
        inline int getIndex()
            {return index;}
        Defect growDendrites();
        Defect getAxon();
        inline double getAxonLength()
            {return axonLength;}
        double getAxonEndToEndDistance();
        inline double getDtreeRadius()
            {return dtreeRadius;}
        Defect getDendrites();
        inline void setKcoreIndex(int idx)
            {kcoreIndex = idx;}
        inline int getKcoreIndex()
            {return kcoreIndex;}
        void printPovRayStructure();
        inline void setRNG(gsl_rng* rngp)
            {rng = rngp;}
        inline std::vector<Vector2d> getAxonSegments()
            {return axonSegments;}
        inline void setAxon(double alen, std::vector<Vector2d> segs)
            {axonLength = alen; axonSegments = segs;}
        inline bool getCUXactive()
            {return CUXactive;}

    private:
        neuron::somaParameters somaParams;
        neuron::axonParameters axonParams;
        neuron::dtreeParameters dtreeParams;
        int index, kcoreIndex;
        float cR, cG, cB;
        Chamber *chamber;
        double somaRadius, dtreeRadius;
        Vector2d position;

        std::vector<Vector2d> axonSegments;
        std::vector<Neuron*> outputConnections;
        std::vector<Neuron*> inputConnections;

        double axonLength;

        gsl_rng* rng;
        bool CUXactive;
};

#endif
    // _NEURON_H_

