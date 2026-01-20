
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
