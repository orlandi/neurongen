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

#include <sys/time.h>			
#include <unistd.h> // for getcwd
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h> // for FILENAMEMAX
#include <string.h>
#include <fstream>
#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"
#include "network.h"
#include <exception>

Network::Network()
{
    init();
}

Network::Network(neuron::chamberParameters p)
{
    init();
    addChamber(p);
}

void Network::init()
{
    chamber = NULL;
}

void Network::addChamber(neuron::chamberParameters p)
{
    if(chamber)
        delete chamber;
    chamber = new Chamber(p);
}

void Network::generate()
{
    seedRNG();
    chamber->assignLattice();
    chamber->assignPatternDefects();

//    chamber->assignDensityMap("densityMatrix.txt", 0.02, 0.02);

    chamber->insertNeurons();
    chamber->growAxons();
    if(inputActive)
    {
        loadPositionalMap(inputPositionsFile);
        loadAxonalMap(inputAxonsFile);
    }

    chamber->growDendrites();
    chamber->growConnections();
}

void Network::activateZone(std::vector<float> zone)
{
    Vector2d center = Vector2d(zone.at(0), zone.at(1));
    Vector2d pos;
    
    for(std::vector<Neuron>::iterator i=chamber->neuron.begin(); i != chamber->neuron.end(); i++)
    {
        pos = i->getPosition();
    }
    chamber->setActiveZone(center, 0.01);
}

void Network::savePositionalMap(std::string fileName)
{
    std::ofstream savedFile(fileName.c_str());
    std::stringstream tmpStr;

    if (!savedFile.is_open())
    {
        std::cout << "There was an error opening file " << fileName;
        return;
    }

    // Logo - file will be used in MATLAB, so use % for non data lines
    savedFile
        << "%-----------------------------------------------------------------\n"
        << "% Neuron11 \n"
        << "% Copyright (c) 2010 Javier G. Orlandi <javiergorlandi@gmail.com> \n"
        << "%-----------------------------------------------------------------\n"
        << "% Generated Positional Map\n"
        << "% Format: Neuron # | X | Y\n"
        << "%-----------------------------------------------------------------\n";

    Vector2d position;
    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i < chamber->neuron.end(); i++)
    {
        position = i->getPosition();
        tmpStr << i-chamber->neuron.begin() << " " << position.x() << " " << position.y() << "\n";
    }

    savedFile << tmpStr.str();
    savedFile.close();
    std::cout << "Positional Map Saved.\n";
}


void Network::saveAxonalMap(std::string fileName)
{
    std::ofstream savedFile(fileName.c_str());
    std::stringstream tmpStr;

    if (!savedFile.is_open())
    {
        std::cout << "There was an error opening file " << fileName;
        return;
    }

    // Logo - file will be used in MATLAB, so use % for non data lines
    savedFile
        << "%-----------------------------------------------------------------\n"
        << "% Neuron11 \n"
        << "% Copyright (c) 2010 Javier G. Orlandi <javiergorlandi@gmail.com> \n"
        << "%-----------------------------------------------------------------\n"
        << "% Generated Positional Map\n"
        << "% Format: Neuron # | Axon Length | N segments | Segments (X,Y) \n"
        << "%-----------------------------------------------------------------\n";

    std::vector<Vector2d> neuronAxonSegments;
    std::vector<double> neuronAxonLength;

    for(std::vector<Neuron>::iterator i=chamber->neuron.begin(); i != chamber->neuron.end(); i++)
    {
        neuronAxonSegments = i->getAxonSegments();
        i->getAxonLength();

        tmpStr << i-chamber->neuron.begin() << " " << i->getAxonLength() << " " << neuronAxonSegments.size() << " ";
        for(std::vector<Vector2d>::iterator j = neuronAxonSegments.begin(); j != neuronAxonSegments.end(); j++)
        {
            tmpStr << j->x() << " " << j->y() << " ";
        }
        tmpStr << "\n";
    }

    savedFile << tmpStr.str();
    savedFile.close();
    std::cout << "Axonal Map Saved.\n";
}   

void Network::loadAxonalMap(std::string fileName)
{
    std::ifstream inputFile;
    std::string line; 

    int nCurrent, nSegments;
    double alength, segX, segY;
    std::vector<Vector2d> segments;

    std::stringstream tmpStr;
    
    inputFile.open (fileName.c_str(), std::ifstream::in);
    if (!inputFile.is_open())
    {
        std::cout << "There was an error opening the file " << fileName << "\n";
        exit(1);
        return;
    }

    while(std::getline(inputFile, line))
    {
        if(line.at(0) == '%')
            continue;
        tmpStr.clear();
        tmpStr.str(line);
        tmpStr >> nCurrent >> alength >> nSegments;
        segments.clear();
        for(int i = 0; i < nSegments; i++)
        {
            tmpStr >> segX >> segY;
            segments.push_back(Vector2d(segX, segY));
        }
        chamber->neuron[nCurrent].setAxon(alength, segments);
    }
}

void Network::loadPositionalMap(std::string fileName)
{
    std::ifstream inputFile;
    std::string line; 

    int nCurrent;
    double posX, posY;

    std::stringstream tmpStr;
    
    inputFile.open (fileName.c_str(), std::ifstream::in);
    if (!inputFile.is_open())
    {
        std::cout << "There was an error opening the file " << fileName << "\n";
        exit(1);
        return;
    }

    while(std::getline(inputFile, line))
    {
        if(line.at(0) == '%')
            continue;
        tmpStr.clear();
        tmpStr.str(line);
        tmpStr >> nCurrent >> posX >> posY;
        chamber->neuron[nCurrent].setPosition(Vector2d(posX, posY));
    }
}

void Network::saveSizes(std::string fileName)
{
    std::ofstream savedFile(fileName.c_str());
    std::stringstream tmpStr;

    if (!savedFile.is_open())
    {
        std::cout << "There was an error opening file " << fileName;
        return;
    }

    // Logo - file will be used in MATLAB, so use % for non data lines
    savedFile
        << "%-----------------------------------------------------------------\n"
        << "% Neuron11 \n"
        << "% Copyright (c) 2010 Javier G. Orlandi <javiergorlandi@gmail.com> \n"
        << "%-----------------------------------------------------------------\n"
        << "% Sizes, lengths and assorted data\n"
        << "% Format: Neuron # | soma radius | dendritic tree radius | axon length | axon end to end distance | # input connections | # output connections";
        if(chamber->getDtreeParameters().CUX)
            savedFile << " | CUX";
        savedFile << "\n%-----------------------------------------------------------------\n";

    Vector2d position;
    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i < chamber->neuron.end(); i++)
    {
        tmpStr << i-chamber->neuron.begin() << " " << i->getSomaRadius() << " " << i->getDtreeRadius() << " "
               << i->getAxonLength() << " " << i->getAxonEndToEndDistance() << " " << i->getInputConnections().size()
               << " " << i->getOutputConnections().size();
        if(chamber->getDtreeParameters().CUX)
            tmpStr << " " << i->getCUXactive();
        tmpStr << "\n";
    }

    savedFile << tmpStr.str();
    savedFile.close();
    std::cout << "Sizes Saved.\n";
}

void Network::saveCUX(std::string fileName)
{
    std::ofstream savedFile(fileName.c_str());
    std::stringstream tmpStr;

    if (!savedFile.is_open())
    {
        std::cout << "There was an error opening file " << fileName;
        return;
    }

    // Logo - file will be used in MATLAB, so use % for non data lines
    savedFile
        << "%-----------------------------------------------------------------\n"
        << "% Neurongen \n"
        << "% Copyright (c) 2012 Javier G. Orlandi <javiergorlandi@gmail.com> \n"
        << "%-----------------------------------------------------------------\n"
        << "% CUX overexpression\n"
        << "% Format: Neuron # | CUX overexpression\n"
        << "%-----------------------------------------------------------------\n";

    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i < chamber->neuron.end(); i++)
    {
        tmpStr << i-chamber->neuron.begin() << " " <<  i->getCUXactive() << "\n";
    }

    savedFile << tmpStr.str();
    savedFile.close();
    std::cout << "CUX Saved.\n";
}

void Network::saveConnections(std::string fileName)
{
    std::ofstream savedFile(fileName.c_str());
    std::stringstream tmpStr;

    if (!savedFile.is_open())
    {
        std::cout << "There was an error opening file " << fileName;
        return;
    }

    // Logo - file will be used in MATLAB, so use % for non data lines
    savedFile
        << "%-----------------------------------------------------------------\n"
        << "% Neuron11 \n"
        << "% Copyright (c) 2010 Javier G. Orlandi <javiergorlandi@gmail.com> \n"
        << "%-----------------------------------------------------------------\n"
        << "% Connection List\n"
        << "% Format: Input | Output\n"
        << "%-----------------------------------------------------------------\n"
        << "% Seed: " << seed << "\n"
        << "%-----------------------------------------------------------------\n";

    std::vector<Neuron*> connections;
    
    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
    {
        connections = i->getOutputConnections();
        for(std::vector<Neuron*>::iterator j = connections.begin(); j != connections.end(); j++)
        {
            tmpStr << i->getIndex() << " " << (*j)->getIndex() << "\n";
        }
        savedFile << tmpStr.str();
        tmpStr.str("");
    }
    savedFile.close();
    std::cout << "Connections saved.\n";
}

void Network::saveConnections2(std::string fileName)
{
    std::ofstream savedFile(fileName.c_str());
    std::stringstream tmpStr;

    if (!savedFile.is_open())
    {
        std::cout << "There was an error opening file " << fileName;
        return;
    }

    // Logo - file will be used in MATLAB, so use % for non data lines
    savedFile
        << "%-----------------------------------------------------------------\n"
        << "% Neuron11 \n"
        << "% Copyright (c) 2010 Javier G. Orlandi <javiergorlandi@gmail.com> \n"
        << "%-----------------------------------------------------------------\n"
        << "% Connection List\n"
        << "% Format: Input | Output\n"
        << "%-----------------------------------------------------------------\n"
        << "% Seed: " << seed << "\n"
        << "%-----------------------------------------------------------------\n";

    std::vector<Neuron*> connections;

    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
    {
        connections = i->getInputConnections();
        for(std::vector<Neuron*>::iterator j = connections.begin(); j != connections.end(); j++)
        {
            tmpStr << (*j)->getIndex() << " " << i->getIndex() << "\n";
        }
        savedFile << tmpStr.str();
        tmpStr.str("");
    }
    savedFile.close();
    std::cout << "Connections saved.\n";
}

std::vector<double> Network::generateKcore()
{
    int currentCore = 1;
    int activeNeighbours;
    int tmpsize = 1;
    bool kcoreUpdated = true;
    std::vector<int> kcoreSize;
    std::vector<double> kcoreRatio;
    Matrix<double, 3, 1> *colList;
    std::vector<Neuron*> neighbours, tmpNeigh;
    kcoreSize.clear();

    std::cout << "Generating the k-core...\n"; 
    // Set everything to 0
    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
        i->setKcoreIndex(0);
    // First select all the neurons that will be checked (the ones of the previous k-core)
    while(tmpsize > 0)
    {
        tmpsize = 0;
        kcoreUpdated = true;
        for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
            if(i->getKcoreIndex() == currentCore-1)
            {
                i->setKcoreIndex(currentCore);
                tmpsize++;
            }
        // Last iteration didn't have any members, quit
        if(tmpsize == 0)
        {
            break;
        }
        else
            kcoreSize.push_back(tmpsize);
        while(kcoreUpdated == true)
        {
            kcoreUpdated = false;
 
            for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
            {
                // Skip neurons that failed some iterations ago
                if(i->getKcoreIndex() != currentCore)
                    continue;
                neighbours = i->getInputConnections();
//                tmpNeigh = i->GetNeighbours();
//                neighbours.insert(neighbours.end(), tmpNeigh.begin(), tmpNeigh.end());

                activeNeighbours = 0;
                for(std::vector<Neuron*>::iterator j = neighbours.begin(); j != neighbours.end(); j++)
                {
                    // If the neighbours survived the last iteration, add them to the list
                    if((*j)->getKcoreIndex() == currentCore)
                        activeNeighbours++;
                }
                // If the amount of neighbours on the previous k-core >= currentCore add this neuron to the new core
                if(activeNeighbours < currentCore)
                {
                    kcoreUpdated = true;
                    i->setKcoreIndex(currentCore-1);
                }
            }
        }
        currentCore++;
    }
    // k-core calculations finished
    std::cout << "K-core size: " << kcoreSize.size() << "\n";
    for(std::vector<int>::iterator i = kcoreSize.begin(); i != kcoreSize.end(); i++)
    {
        std::cout << *i << " ";
        kcoreRatio.push_back(double(*i)/chamber->neuron.size());
    }
 
    std::cout << "\n";
    return kcoreRatio;
}

std::vector<double> Network::generateOutputKcore()
{
    int currentCore = 1;
    int activeNeighbours;
    int tmpsize = 1;
    bool kcoreUpdated = true;
    std::vector<int> kcoreSize;
    std::vector<double> kcoreRatio;
    Matrix<double, 3, 1> *colList;
    std::vector<Neuron*> neighbours, tmpNeigh;
    kcoreSize.clear();

    std::cout << "Generating the output k-core...\n"; 
    // Set everything to 0
    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
        i->setKcoreIndex(0);
    // First select all the neurons that will be checked (the ones of the previous k-core)
    while(tmpsize > 0)
    {
        tmpsize = 0;
        kcoreUpdated = true;
        for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
            if(i->getKcoreIndex() == currentCore-1)
            {
                i->setKcoreIndex(currentCore);
                tmpsize++;
            }
        // Last iteration didn't have any members, quit
        if(tmpsize == 0)
        {
            break;
        }
        else
            kcoreSize.push_back(tmpsize);
        while(kcoreUpdated == true)
        {
            kcoreUpdated = false;
 
            for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
            {
                // Skip neurons that failed some iterations ago
                if(i->getKcoreIndex() != currentCore)
                    continue;
                neighbours = i->getOutputConnections();
//                tmpNeigh = i->GetNeighbours();
//                neighbours.insert(neighbours.end(), tmpNeigh.begin(), tmpNeigh.end());

                activeNeighbours = 0;
                for(std::vector<Neuron*>::iterator j = neighbours.begin(); j != neighbours.end(); j++)
                {
                    // If the neighbours survived the last iteration, add them to the list
                    if((*j)->getKcoreIndex() == currentCore)
                        activeNeighbours++;
                }
                // If the amount of neighbours on the previous k-core >= currentCore add this neuron to the new core
                if(activeNeighbours < currentCore)
                {
                    kcoreUpdated = true;
                    i->setKcoreIndex(currentCore-1);
                }
            }
        }
        currentCore++;
    }
    // k-core calculations finished
    std::cout << "K-core size: " << kcoreSize.size() << "\n";
    for(std::vector<int>::iterator i = kcoreSize.begin(); i != kcoreSize.end(); i++)
    {
        std::cout << *i << " ";
        kcoreRatio.push_back(double(*i)/chamber->neuron.size());
    }
    std::cout << "\n";
    return kcoreRatio;
}

std::vector<double> Network::generateUndirectedKcore()
{
    int currentCore = 1;
    int activeNeighbours;
    int tmpsize = 1;
    bool kcoreUpdated = true;
    std::vector<int> kcoreSize;
    std::vector<double> kcoreRatio;
    Matrix<double, 3, 1> *colList;
    std::vector<Neuron*> neighbours, tmpNeigh;
    kcoreSize.clear();

    std::cout << "Generating the undirected k-core...\n"; 
    // Set everything to 0
    for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
        i->setKcoreIndex(0);
    // First select all the neurons that will be checked (the ones of the previous k-core)
    while(tmpsize > 0)
    {
        tmpsize = 0;
        kcoreUpdated = true;
        for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
            if(i->getKcoreIndex() == currentCore-1)
            {
                i->setKcoreIndex(currentCore);
                tmpsize++;
            }
        // Last iteration didn't have any members, quit
        if(tmpsize == 0)
        {
            break;
        }
        else
            kcoreSize.push_back(tmpsize);
        while(kcoreUpdated == true)
        {
            kcoreUpdated = false;
 
            for(std::vector<Neuron>::iterator i = chamber->neuron.begin(); i != chamber->neuron.end(); i++)
            {
                // Skip neurons that failed some iterations ago
                if(i->getKcoreIndex() != currentCore)
                    continue;
                neighbours = i->getInputConnections();
                tmpNeigh = i->getOutputConnections();
                neighbours.insert(neighbours.end(), tmpNeigh.begin(), tmpNeigh.end());

                activeNeighbours = 0;
                for(std::vector<Neuron*>::iterator j = neighbours.begin(); j != neighbours.end(); j++)
                {
                    // If the neighbours survived the last iteration, add them to the list
                    if((*j)->getKcoreIndex() == currentCore)
                        activeNeighbours++;
                }
                // If the amount of neighbours on the previous k-core >= currentCore add this neuron to the new core
                if(activeNeighbours < currentCore)
                {
                    kcoreUpdated = true;
                    i->setKcoreIndex(currentCore-1);
                }
            }
        }
        currentCore++;
    }
    // k-core calculations finished
    std::cout << "K-core size: " << kcoreSize.size() << "\n";
    for(std::vector<int>::iterator i = kcoreSize.begin(); i != kcoreSize.end(); i++)
    {
        std::cout << *i << " ";
        kcoreRatio.push_back(double(*i)/chamber->neuron.size());
    }
  
    std::cout << "\n";
    return kcoreRatio;
}

bool Network::seedRNG()
{
    std::stringstream tmpStr, seedStr;
	struct timeval tv;
	gettimeofday(&tv,NULL);
    struct tm *tm = localtime(&tv.tv_sec);
	seed = abs(int(tv.tv_usec/10+tv.tv_sec*100000));	// Creates the seed based on actual time
	
    rng = gsl_rng_alloc(gsl_rng_taus2);
	
    gsl_rng_set(rng,seed);			// Seeds the previously created RNG
    chamber->setRNG(rng);
/*
    tmpStr << "% Date: " << tm->tm_mday << "/" << tm->tm_mon +1 << "/" << tm->tm_year + 1900 << ", "
           << "Time: " << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec << ", "
           << "RNG Seed: " << seed << "\n";
    // EEW
    char currentPath[FILENAME_MAX];
    char seedChar [FILENAME_MAX];
    char tmpChar [FILENAME_MAX];

    sprintf(seedChar,"%d",seed);
    getcwd(currentPath, FILENAME_MAX);
    strcpy(tmpChar,"../data");
    strcat(tmpChar,"/");
    strcat(tmpChar,seedChar);

    mkdir("../data",0777);
    
    if(mkdir(tmpChar,0777)==-1) //creating a directory
    {
        std::cerr << "Warning:  " << strerror(errno) << std::endl;
    }
    strcat(currentPath,"/");
    strcat(currentPath, tmpChar);
    chdir(currentPath);

    seedStr << seed << ".txt";
    savedFileName = seedStr.str();

    InitSaveResults(tmpStr.str());*/
    return true;
}


void Network::loadConfigFile(std::string filename)
{
    std::string tmpStr;
    bool generateNetwork = false;
    neuron::chamberParameters cparams = neuron::DEFAULT_CHAMBER_PARAMETERS;
    neuron::cultureParameters cultparams = neuron::DEFAULT_CULTURE_PARAMETERS;
    neuron::somaParameters somaparams = neuron::DEFAULT_SOMA_PARAMETERS;
    neuron::dtreeParameters dtreeparams = neuron::DEFAULT_DTREE_PARAMETERS;
    neuron::axonParameters axonparams = neuron::DEFAULT_AXON_PARAMETERS;

    configFile = new libconfig::Config();
    try
    {
        configFile->readFile(filename.c_str());
    }
    catch(libconfig::ParseException e)
    {
        std::cout << e.getLine() << " " << e.getError() << "\n";
    }
    configFile->setAutoConvert(true);
    // Start setting the connectivity
    if(!configFile->lookupValue("network.generation", generateNetwork))
    {
        std::cout << "Main error. Network generation variable not set\n";
        exit(1);
    }

    // Generate the network
    if(generateNetwork)
    {
        if(!configFile->lookupValue("network.pattern.file", tmpStr))
        {
            std::cout << "Main error. Network pattern file not set\n";
            exit(1);
        }
        // Assign the pattern
        cparams.pattern = true;
        cparams.patternFile = tmpStr;
        cparams.type = neuron::CH_TYPE_CUSTOM;
        cparams.units = neuron::UNITS_MILIMETERS; 
        if(!configFile->lookupValue("network.pattern.width", cparams.width))
            std::cout << "Warning! Missing network.pattern.width\n";
        if(!configFile->lookupValue("network.pattern.height", cparams.height))
            std::cout << "Warning! Missing network.pattern.height\n";
        
        // Create the chamber
        addChamber(cparams);
        
        // Add neurons
        if(!configFile->lookupValue("network.neurons", cultparams.neuronNumber))
            std::cout << "Warning! Missing network.neurons\n";
        setCultureParameters(cultparams);

        // Configure neurons PARTIALLY MISSING
        // Configure soma
        if(!configFile->lookupValue("network.soma.radius", somaparams.radius))
            std::cout << "Warning! Missing network.soma.radius\n";

        // Configure dendritic tree

        if(!configFile->lookupValue("network.dendritic_tree.type", tmpStr))
            std::cout << "Warning! Missing network.dendritic_tree.type\n";
        if(!tmpStr.compare("homogeneous"))
                dtreeparams.type = neuron::DTREE_TYPE_HOMOGENEOUS;
        else
            std::cout << "Warning! Invalid network.dendritic_tree.type\n";
        
        if(!configFile->lookupValue("network.dendritic_tree.shape", tmpStr))
            std::cout << "Warning! Missing network.dendritic_tree.shape\n";
        if(!tmpStr.compare("circular"))
                dtreeparams.shape = neuron::DTREE_SHAPE_CIRCULAR;
        else
            std::cout << "Warning! Invalid network.dendritic_tree.shape\n";

        if(!configFile->lookupValue("network.dendritic_tree.radius_distribution", tmpStr))
            std::cout << "Warning! Missing network.dendritic_tree.radius_distribution\n";
        if(!tmpStr.compare("gaussian"))
                dtreeparams.sizeDistribution = neuron::DISTRIBUTION_GAUSSIAN;
        else
            std::cout << "Warning! Invalid network.dendritic_tree.radius_distribution\n";
 
        if(!configFile->lookupValue("network.dendritic_tree.radius_mean", dtreeparams.meanRadius))
            std::cout << "Warning! Missing network.dendritic_tree.radius_mean\n";
        if(!configFile->lookupValue("network.dendritic_tree.radius_std_dev", dtreeparams.stdRadius))
            std::cout << "Warning! Missing network.dendritic_tree.std_dev\n";
 
        // Configure axons

        // Axon type
        if(!configFile->lookupValue("network.axon.type", tmpStr))
            std::cout << "Warning! Missing network.axon.type\n";
        if(!tmpStr.compare("segmented"))
                axonparams.type = neuron::AXON_TYPE_SEGMENTED;
        else
            std::cout << "Warning! Invalid network.axon.type\n";
        // Axon distribution
        if(!configFile->lookupValue("network.axon.length_distribution", tmpStr))
            std::cout << "Warning! Missing network.axon.length_distribution\n";
        if(!tmpStr.compare("rayleigh"))
                axonparams.lengthDistribution = neuron::DISTRIBUTION_RAYLEIGH;
        else
            std::cout << "Warning! Invalid network.axon.length_distribution\n";

        if(!configFile->lookupValue("network.axon.length_mean", axonparams.meanLength))
            std::cout << "Warning! Missing network.axon.length_mean\n";
        
        if(!configFile->lookupValue("network.axon.length_std_deviation", axonparams.stdLength))
            std::cout << "Warning! Missing network.axon.length_std_deviation\n";
        
        if(!configFile->lookupValue("network.axon.width", axonparams.width))
            std::cout << "Warning! Missing network.axon.width\n";

        // Initial Angle distribution
        if(!configFile->lookupValue("network.axon.initial_angle_distribution", tmpStr))
            std::cout << "Warning! Missing network.axon.initial_angle_distribution\n";
        if(!tmpStr.compare("uniform"))
            axonparams.initialAngleDistribution = neuron::DISTRIBUTION_UNIFORM;
        else
            std::cout << "Warning! Invalid network.axon.initial_angle_distribution\n";

        if(!configFile->lookupValue("network.axon.initial_angle_mean", axonparams.meanInitialAngle))
            std::cout << "Warning! Missing network.axon.initial_angle_mean\n";

        if(!configFile->lookupValue("network.axon.initial_angle_std_dev", axonparams.stdInitialAngle))
            std::cout << "Warning! Missing network.axon.initial_angle_std_dev\n";

        // Segment Angle distribution
        if(!configFile->lookupValue("network.axon.segment_angle_distribution", tmpStr))
            std::cout << "Warning! Missing network.axon.segment_angle_distribution\n";
        if(!tmpStr.compare("gaussian"))
            axonparams.initialAngleDistribution = neuron::DISTRIBUTION_GAUSSIAN;
        else
            std::cout << "Warning! Invalid network.axon.segment_angle_distribution\n";

        if(!configFile->lookupValue("network.axon.segment_angle_mean", axonparams.meanSegmentAngle))
            std::cout << "Warning! Missing network.axon.segment_angle_mean\n";

        if(!configFile->lookupValue("network.axon.segment_angle_std_dev", axonparams.stdSegmentAngle))
            std::cout << "Warning! Missing network.axon.segment_angle_std_dev\n";

        if(!configFile->lookupValue("network.axon.segment_angle_max_std_dev", axonparams.maxStdSegmentAngle))
            std::cout << "Warning! Missing network.axon.segment_angle_max_std_dev\n";

        if(!configFile->lookupValue("network.axon.segment_length", axonparams.segmentLength))
            std::cout << "Warning! Missing network.axon.segment_length\n";

        if(!configFile->lookupValue("network.axon.segment_count", axonparams.segmentCount))
            std::cout << "Warning! Missing network.axon.segment_count\n";

        if(!configFile->lookupValue("network.axon.segment_max_retries", axonparams.maxRetries))
            std::cout << "Warning! Missing network.axon.segment_max_retries\n";

        // Segment type
        if(!configFile->lookupValue("network.axon.segment_type", tmpStr))
            std::cout << "Warning! Missing network.axon.segment_type\n";
        if(!tmpStr.compare("fixed length"))
            axonparams.segmentType = neuron::AXON_STYPE_FIXEDLENGTH;
        else
            std::cout << "Warning! Invalid network.axon.segment_type\n";
         
        // Collision Mode
        if(!configFile->lookupValue("network.axon.collision_mode", tmpStr))
            std::cout << "Warning! Missing network.axon.collision_mode\n";
        if(!tmpStr.compare("pattern"))
        {
            axonparams.collisionFlags = 0;
            axonparams.collisionFlags = neuron::COL_PATTERN;
        }
        else
            std::cout << "Warning! Invalid network.axon.collision_mode\n";

        

        // Configure CUX
        if(!configFile->lookupValue("network.CUX.active", dtreeparams.CUX))
        {
            std::cout << "Warning! Missing network.CUX.active\n";
            dtreeparams.CUX = false;
        }
        if(dtreeparams.CUX)
        {
            dtreeparams.sizeDistribution = neuron::DISTRIBUTION_CUX;
            if(!configFile->lookupValue("network.CUX.fraction", dtreeparams.CUXfraction))
                std::cout << "Warning! Missing network.CUX.fraction\n";
            if(!configFile->lookupValue("network.CUX.dendritic_tree_multiplier", dtreeparams.CUXmultiplier))
                std::cout << "Warning! Missing network.CUX.dendritic_tree_multiplier\n";
            if(!configFile->lookupValue("network.CUX.file", CUXfile))
                std::cout << "Warning! Missing network.CUX.file\n";
        }

        // Set all the parameters in place
        setNeuronParameters(somaparams, dtreeparams, axonparams);
        // set the inputs
        
        if(!configFile->lookupValue("network.input.active", inputActive))
        {
            std::cout << "Warning! Missing network.input.active\n";
            inputActive = false;
        }
        if(inputActive)
        {
            if(!configFile->lookupValue("network.input.positions_file", inputPositionsFile))
                std::cout << "Warning! Missing network.input.positions_file\n";
            if(!configFile->lookupValue("network.input.axons_file", inputAxonsFile))
                std::cout << "Warning! Missing network.input.axons_file\n";
        }

        // Finally generate the network
        generate();

        // Save everything
        if(!configFile->lookupValue("network.output.positions", tmpStr))
            std::cout << "Warning! Missing output.positions - Not saving file\n";
        else
            savePositionalMap(tmpStr);
        if(!configFile->lookupValue("network.output.axons", tmpStr))
            std::cout << "Warning! Missing output.axons - Not saving file\n";
        else
            saveAxonalMap(tmpStr);
        if(!configFile->lookupValue("network.output.connections", tmpStr))
            std::cout << "Warning! Missing output.connections - Not saving file\n";
        else
            saveConnections(tmpStr);
        if(!configFile->lookupValue("network.output.sizes", tmpStr))
            std::cout << "Warning! Missing output.sizes - Not saving file\n";
        else
            saveSizes(tmpStr); 

        // Save CUX
        if(dtreeparams.CUX)
            saveCUX(CUXfile);        
    }
}

