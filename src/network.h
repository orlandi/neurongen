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

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <libconfig.h++>
#include "neuronnamespace.h"
#include "chamber.h"

class Network
{
    public:
        Network();
        Network(neuron::chamberParameters p);
        void addChamber(neuron::chamberParameters p = neuron::DEFAULT_CHAMBER_PARAMETERS);
        inline void setCultureParameters(neuron::cultureParameters cparam)
            {chamber->setCultureParameters(cparam);}
        inline void setNeuronParameters(neuron::somaParameters sparam, neuron::dtreeParameters dparam, neuron::axonParameters aparam)
            {chamber->setNeuronParameters(sparam, dparam, aparam);}
        void generate();
        void activateZone(std::vector<float> zone);
        std::vector<double> generateKcore();
        std::vector<double> generateOutputKcore();
        std::vector<double> generateUndirectedKcore();

        void savePositionalMap(std::string fileName);
        void loadPositionalMap(std::string fileName);
        void loadAxonalMap(std::string fileName);
        void saveAxonalMap(std::string fileName);
        void saveConnections(std::string fileName);
        void saveConnections2(std::string fileName);
        void saveSizes(std::string fileName);
        void saveCUX(std::string fileName);
        void saveGexf(std::string fileName);
        bool seedRNG();

        void loadConfigFile(std::string filename);

    private:
        void init();
        Chamber* chamber;
        int seed;
        gsl_rng* rng;
        libconfig::Config* configFile;
        bool inputActive;
        std::string inputAxonsFile, inputPositionsFile, CUXfile, gexfFile;
};

#endif
    // _NETWORK_H_

