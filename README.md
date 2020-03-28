# B.Harvestr
Repository: BHarvestr

Description: B.Harvestr is an **experimental** granular synthesizer LV2 plugin.

**Warning: B.Harvestr is in an early stage of development. Not for pruducion use! It is not
stable yet. Some essential features including saving and restoring the plugin state are not
(fully) implemented yet. Major changes in the plugin definition need to be expected. Therefore,
future versions of this plugin may be completely incompatible to this version.**


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


## Usage

## TODO

* Save / restore plugin state features: user pattern, user shapes
* Extend MIDI support / features
* Pitch detection
* Add process features (filters, reverb, ...)
* Use two dials to control pitch (st and ct)
* Adjust output level to nr of grains
* Reduce CPU load !!!
* Move notification handler to worker process
* Write documentation
* Create presets


## Links
