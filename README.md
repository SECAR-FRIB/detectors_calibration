
# Root scripts for detectors calibration and simple analysis tasks
This repository contains scripts that are used for calibrating the various detectors of SECAR system. It also contains a script for plotting traces

## Requirements
For using the scripts the data must be in ROOT format either raw or sorted. Use the codes from [analysis_root repo]([url](https://github.com/SECAR-FRIB/analysis_root)) to convert the evt files to root.

### DSSD calibration
  * Run first the DSSD_AnalyseSpectra.C for finding the peaks' channel location per strip for a calibration run (read the dedicated readme for more details)
    * run as root -l DSSD_AnalyseSpectra.C
    * creates txt file with peak location and uncertainty in channels
    * saves PNG images of the peak fitting (optional)
  * Use the txt output of the DSSD_AnalyseSpectra.C as input to the calibrate_DSSD.C script to obtain per strip calibration parameters
    * run as rroot -l calibrate_DSSD.C
    * creates a txt file with calibration parameters per strip
    * saves PNG images of calibration per strip  
  * Use the convert_DSSDcal_SpectclVar.py script for converting the calibration parameters to Spectcl format
    * creates a txt file with the calibration parameters per script in Spectcl definition file format 

### TAC calibration
  * Use calibrate_TAC.C script to obtain TAC calibration
    * creates a PNG of TAC histogram with marks on peaks 
    * creates a PNG of TAC calibration
    * prints calibration parameters on terminal

### Plot traces

#### `plot_traces.C`

ROOT macro for quickly plotting **DDAS digitizer traces** (ADC samples vs sample index) from a SECAR “dumpedfiles” ROOT file.

It opens a ROOT file, reads the `dchan` TTree, loops over `ddasevent` entries, selects a specific **crate / module(slot) / channel**, pulls the waveform trace (`GetTrace()`), and overlays up to `maxTracesToPlot` traces on a canvas.

#### How to run

From a terminal:

##### Interactive ROOT session

```bash
root -l
```
In ROOT:

```cpp
.L plot_traces.C
plot_traces();
```
##### One-liner (batch-ish)

```bash
root -l plot_traces.C
```

#### What you should edit

Open `plot_traces.C` and adjust these knobs near the top of `plot_traces()`:

##### 1) Input file path
This script uses the "raw" ROOT file. To create it run the create_rawROOT.sh from the sort_runs.sh script that is in the [root analysis repo]([url](https://github.com/SECAR-FRIB/analysis_root))

```cpp
TFile* pFile = new TFile("/mnt/analysis/{exp_number}/dumpedfiles/run{number}.root");
```

##### 2) Channel selection (crate / module / channel)

Crate 0 i the target, crate 1 is the FP4 detectors. Modules start from 2 (0 and 1 are reserrved for the PC) so for example if the daq map file shows a detector on module 0 here it is 2, or if it is at 3 it corresponds to 5. That's why there is a +2 on the plotMod variable.

```cpp
int plotCrate = 1; // 0 for target and 1 for FP4 (per comment in the script)
int plotMod   = 0+2; // note: modules 0 and 1 reserved in these scripts
int plotChan  = 12;
```

##### 3) How many traces to overlay

Choose how many traces you want to overlay on one plot, each will appear with different color

```cpp
const int maxTracesToPlot = 10;
```

##### Output

* A `TCanvas` titled **"Traces"** with up to `maxTracesToPlot` traces overlaid.
* The macro prints how many traces were plotted:

```text
Plotted N traces.
```
