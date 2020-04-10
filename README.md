# B.Harvestr
Repository: BHarvestr

Description: B.Harvestr is an **experimental** granular synthesizer LV2 plugin.

**Warning: B.Harvestr is in an early stage of development.
Not for production use! No guarantees! Some essential features are not (fully) implemented yet.
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
as (the majority of) pitch shifters. Granular synthesizers typically generate lots of overlapping
grains to produce atmospheric sound effects, full soundscapes and pad sounds.

Starting point for granular synthesis is a sample. In theory, any kind of sample can be used. The sample
is typically chopped by the granular synthesizer into fragments of 1 to 100 ms or sometimes even longer.
These fragments are called grains. The grains are modulated and re-arranged to produce a new sound
experience that may strongly differ from the initial sample.

![grains](https://raw.githubusercontent.com/sjaehn/BHarvestr/master/doc/grains.png "Concept of granular synthesis in B.Harvestr")


## Usage

### Quick guide

#### Step 1 : Load a sample (or use the default sine wave)

Click on the file open symbol or the file name to select an audio source file.

#### Step 2 : Select a region

Drag the rulers in the sample window to select a region to use for granular synthesis. It is recommended to
exclude the starting and the and region of the sample.

Also assign a note or a frequency to the selected region (default: A4 / 440 Hz).

#### Step 3 : Set grain size and/or grain rate

Change the grain size or rate by dragging or scrolling *inside* the ring of the respective dial. Shortening
of the grain size may result in a deviation from the original sound. Decreasing the grain rate results in a
staccato effect whereas increasing the rate may produce a fuller sound. Especially if some
modulation is added.

#### Step 4 : Apply modulation

Set a modulation range for the grain properties by dragging or scrolling *outside* the ring of the respective
dial. Click on the down button next to the respective dial and select up to four modulators for each property.

You can select between four envelopes, four LFOs, four sequencers, and four random engines. Randomness
applied to grain properties may also prevent unwanted resonance produced by symmetrically overlaying grains.

Modulation can also be applied to the process properties. Process level needs to be connected to at least
one envelope modulator (default: Env1).


## TODO

* Extend MIDI support / features
* Pitch detection
* Add process features (filters, reverb, ...)
* Notify modulates process to GUI
* Find a better solution for the 1024 pattern steps limit
* Adjust output level to nr of grains
* Reduce CPU load !!!
* Use GPU for GUI effects
* Move notification handler to worker process
* Enable multi-line text edit
* Improve modulator selection dialog
* Keyboard mapping
* Write documentation
* Create presets


## Links
