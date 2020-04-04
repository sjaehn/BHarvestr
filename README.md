# B.Harvestr
Repository: BHarvestr

Description: B.Harvestr is an **experimental** granular synthesizer LV2 plugin.

**Warning: B.Harvestr is in an early stage of development. It still produces a heavy CPU load.
Not for pruducion use! No guarantees! Some essential features are not (fully) implemented yet.
Major changes in the plugin definition need to be expected. Therefore, future versions of this
plugin may be completely incompatible to this version.**


## Installation

Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Install pkg-config and the development packages for sndfile, x11, cairo, and lv2 if not
done yet. On Debian-based systems you may run:
```
sudo apt-get install pkg-config libsndfile-dev libx11-dev libcairo2-dev lv2-dev
```

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
make
sudo make install PREFIX=/usr
```
from the directory where you downloaded the repository files. For installation into an
alternative directory (e.g., /usr/local/lib/lv2/), change the variable `PREFIX` while installing:

```
sudo make install PREFIX=/usr/local
```

## Running

After the installation Carla, Ardour and any other LV2 host should automatically detect B.Harvestr.

If jalv is installed, you can also call it
```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BHarvestr
```
to run it stand-alone and connect it to the JACK system.


## Granular synthesis

In contrast to standard synthesis methods which are based on oscillating waves, granular synthesis
uses micro fragments of audio samples to generate sound. Thus, granular synthesis uses the same concept
as (the majority of) pitch shifters. Granular synthesiziers typically generate lots of overlapping
grains to produce athmospheric sound effects, full soundscapes and pad sounds.

Starting point for granular synthesis is a sample. In theory, any kind of sample can be used. This sample
is typically chopped by the granular synthesizer into fragments of 1 to 100 ms or even longer. These
fragments are called grains. The grains are modulated and re-arranged to produce a new sound experience
that may strongly differ from the initial sample.

![grains](https://raw.githubusercontent.com/sjaehn/BHarvestr/master/doc/grains.png "Concept of granular synthesis in B.Harvestr")


## Usage

### Quick guide

A good starting point is: load a sample. Alternatively, you also may use the default sine wave sample.
Select a region of the sample by dragging the rulers. Change the size of the grains if you like. Add some
randomness to the grain rate and/or grain phase and/or grain level and/or grain pitch by clicking the down
button next to the respective dial and selection of a modulator (e.g., Rnd1).


## TODO

* Extend MIDI support / features
* Pitch detection
* Add process features (filters, reverb, ...)
* Find a better solution for the 1024 pattern steps limit
* Adjust output level to nr of grains
* Reduce CPU load !!!
* Move notification handler to worker process
* Enable multi-line text edit
* Improve modulator selection dialog
* Keyboard mapping
* Write documentation
* Create presets


## Links
