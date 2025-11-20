# detectors_calibration
Root scripts for detectors calibration
* DSSD calibration
  * Run first the DSSD_AnalyseSpectra.C for finding the peaks' channel location per strip for a calibration run
  * Use the txt output of the DSSD_AnalyseSpectra.C as input to the calibrate_DSSD.C script to obtain per strip calibration parameters
  * Use the convert_DSSDcal_SpectclVar.py script for converting the calibration parameters to Spectcl format

* TAC calibration
  * Use calibrate_TAC.C script to obtain TAC calibration 
