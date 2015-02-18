# Generates realistic neuronal networks using arbitrary geometrical patterns as a substrate


## Dependencies

Qt4
gsl
eigen3

## Step by step installation guide

### Ubuntu-based systems (tested on 64-bit Mint 17)

If you are on a fresh install of ubuntu server you might need some very basic stuff, like make, rake, gcc, g++ and git, so install them:
  
    sudo apt-get install make gcc g++ git

Now let's start with the real dependencies, Qt4, eigen3, gsl

    sudo apt-get install qt-sdk libeigen3-dev libgsl0-dev libconfig++-dev 

We should be set, now let's clone this repository:

    cd ~
    git clone https://github.com/orlandi/neurongen

Now let's run qmake to create the makefiles

    cd neurongen
    qmake 

And compile with make

    make

We are set. If everything went ok you should have the 'neurongen' executable

## Usage

The program reads the file config.cfg located in the same folder and
generates a network given the parameters specified in that file. Check
the comments on that file to know the available options.

Pay special attention to how the pattern is defined. It consist of a png
2-bit image (black&white) indicating where a neuron can grow. Black =
allowed. White = foribdden. Neurons will only be placed and grow their
processes in the black pixels. If they find a white pixel on their path
they will try to avoid it.

Note that the program assumes periodic boundary conditions (toroidal),
so if you want to use normal boundary conditions, add a white pixel to
the borders of your image.

Some basic patterns are provided in the patterns folder


The default config.cfg file looks something like this:
```
# Default configuration file
# Note, all metric units are in mm

version = 1.0;

network:
{
    generation = true;       # true/false. If false it uses the files
specified
                             # in the input section to generate a new
network
                             # (this is experimental)

    pattern:                 # Pattern (substrate) parameters
    {
        file = "patterns/circ.png";   # Input file used to generate the pattern.
                             # This file is a 2-bit png image (black &
white)
                             # That tells the program where neurons can
grow:
                             # (Black pixel = allowed / white pixel =
forbidden)
                             # Note that the program assumes periodic
boundary 
                             # conditions (toroidal). If you want to set
non-periodic
                             # boundary conditions just make the pixels
in the border
                             # white
        width = 10.0;
        height = 10.0;       # Width and height of the culture. The size
of each pixel
                             # comes from dividing these values by the
number of pixels
                             # in each dimension of the image. Ex. The
circ.png image is
                             # 350x350 pixels. This corresponds to a
pixel size of 
                             # ~0.03x0.03 mm. Note that if you want to
use complex geometries
                             # you should set a pixel size bigger than
the dendritic tree
                             # or you will run into bugs.
    };
    neurons = 20000;         # Number of neurons
    soma:
    {
        radius = 0.0075;     # Soma radius (remember, in mm)
    };
    dendritic_tree:
    {
        type = "homogeneous"; # (homogeneous/fractal) Type of dendritic
tree structure.
                              # fractal was never implemented.
        shape = "circular";   # Shape of the dentric tree (only circular
was implemented)
        radius_distribution = "gaussian"; # Distribution used to
generate the sizes 
                              #(gaussian/rayleigh)
        radius_mean = 0.15;
        radius_std_dev = 0.02; # mean and std deviation parameters of
the chosen distribution
    };
    axon:
    {
        type = "segmented";   # Axon parameters, I will finish comenting
them later.
                              # If you want more details look at
neuron.cc and
                              # the neuronnamespace.h
        length_distribution = "rayleigh";
        length_mean = 0;
        length_std_deviation = 0.8;
        width = 0.001;
        initial_angle_distribution = "uniform";
        initial_angle_mean = 0;
        initial_angle_std_dev = 0;
        segment_angle_distribution = "gaussian";
        segment_angle_mean = 0;
        segment_angle_std_dev = 0.1;
        segment_angle_max_std_dev = 3.2;
        segment_length = 0.01;
        segment_count = 20;
        segment_max_retries = 10;
        # Either "fixed length" or "fixed count"
        segment_type = "fixed length";
        # Can be "pattern" "soma" "axon"
        collision_mode = "pattern";
    };
    # Experimental, forget about CUX
    CUX:
    {
        active = false;
        fraction = 0.05;
        dendritic_tree_multiplier = 1.5;
        file = "cux.txt";
    };
    # Output files generated by the program (see the files headers for
structure)
    output:
    {
      positions = "map.txt";      # Contains the positions of the
neurons
      axons = "axons.txt";        # Contains the positions of each axon
      connections = "cons.txt";   # List of connections formed
(adjacency matrix)
      sizes = "sizes.txt";        # List of size parameters of each
neuron
        gexf = "network.gexf";      # The network in gexf format to load
in gephi
    };
    # Input files used in case you do not generate the network
(experimental)
    input:
    {
        active = false;
        positions_file = "map.txt";
        axons_file = "axons.txt";
    };
};
```

## Known bugs

Use a pixel size higher than the dendritic tree size, otherwise neurons
might make connections across barriers

