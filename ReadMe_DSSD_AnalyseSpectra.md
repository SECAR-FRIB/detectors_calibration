
# DSSD_AnalyseSpectra_v2.C

ROOT macro to analyse DSSD spectra strip-by-strip, fit Gaussian peaks, and save peak positions for calibration.

## 1. What it does

* Loops over all **64 channels** of a DSSD (4 modules × 16 channels).
* For each channel:

  * Draws the **raw silicon energy spectrum** `energy_Si_raw[i]`.
  * Optionally rebins (currently disabled by default).
  * **Skips** spectra with too few counts.
  * Fits either:

    * a **single Gaussian** with **automatic asymmetric limits** (using a fraction of the peak height), or
    * a **double Gaussian** with **manual limits** for each peak.
  * Saves a PNG of the spectrum + fit per strip.
  * Writes the fitted peak position(s) and error(s) to a text file.

---

## 2. Prerequisites

* ROOT (macro is written as a ROOT C++ script).
* Input (sorted) ROOT file with:

  * a `TTree` named `"t"`.
  * a branch `energy_Si_raw[64]` (one entry per DSSD channel).
* A DSSD map file `DSSD_map_2024.dat` containing, per line:

  ```text
  mod  chan  strip  side
  ```

---

## 3. User-configurable settings

```cpp
runNum = 2035;  // Select a run

// Input file configuration
std::string inputDir  = "/mnt/analysis/e20008/rootfiles"; // directory containing ROOT files
std::string inputPrefix  = "run"; // prefix BEFORE run number (e.g., run2035-00.root)
ifstream DSSDMap("DSSD_map_2024.dat"); // DSSD strip map file

// Output configuration
std::string outputDir   = ".";            // directory for output text file
std::string outputPrefix = "DSSD_calib_output_run";
std::string outputTag    = "_Nov25";      // version/tag appended to filename
std::string plotsDir     = "plots";       // directory where PNGs will be saved

// Spectrum formatting
min  = 1;           // minimum histogram channel
max  = 4000;        // maximum histogram channel
bins = (max-min)/10;
int min_counts = 100;   // minimum counts required to attempt a fit

// Fit settings
int fitMode = 1;    // 1 = single Gaussian (auto limits), 2 = double Gaussian (manual)
double leftLim  = 0.7; // left-side threshold (fraction of peak height)
double rightLim = 0.2; // right-side threshold (fraction of peak height)
bool autoFit = true;   // disable interactive input per strip
```
---

## 4. Single Gaussian mode (fitMode = 1)

* The helper `FindPeakAndLimits`:

  * Finds the maximum bin of the histogram.
  * Computes thresholds:

    * left side:  `leftLim  × maxContent`
    * right side: `rightLim × maxContent`
  * Walks left and right from the peak until bin content drops below these thresholds.
  * Uses the resulting `[xLow, xHigh]` as the fit range.

* A Gaussian (`TF1("g1","gausn",xLow,xHigh)`) is fitted in that range.

* The script saves:

  * `peak_1` = Gaussian mean,
  * `sigma_peak_1` = error on the mean.

In **single-Gaussian mode**, the output line contains:

```text
mod   chan   peak_1   sigma_peak_1
```

---

## 5. Double Gaussian mode (fitMode = 2)

Per strip, you are prompted for:

```text
lower limit of first peak
upper limit of first peak
lower limit of second peak
upper limit of second peak
```

The macro then:

1. Fits two separate Gaussians (`g1`, `g2`) in the two ranges.

2. Builds a **double Gaussian**:

   ```cpp
   TF1 *dg = new TF1("dg","gausn(0)+gausn(3)", lim1, lim4);
   ```

   initialized from `g1` and `g2` parameters.

3. Fits `dg` over `[lim1, lim4]` and extracts:

   * `peak_1`, `sigma_peak_1` from the first Gaussian,
   * `peak_2`, `sigma_peak_2` from the second.

In **double-Gaussian mode**, the output line contains:

```text
mod   chan   peak_1   sigma_peak_1   peak_2   sigma_peak_2
```

---

## 6. Skipping channels and low statistics

* If the integral in `[min, max]` is **< 100 counts**, the strip is considered **low statistics** and is skipped.
* If `autoFit == false`, you can also manually choose to **skip** a spectrum.

In both cases, the script writes zeros for that channel in the output file.

---

## 7. Output

1. **Text file** (append mode):

   * Name: `output<runNum>_DSSD_Nov25.txt`
   * Contains one line per DSSD channel with the fitted peak position(s).

2. **PNG plots**:

   * Saved per strip under `plots/` as
     `run<runNum>_strip<strip>_F.png` (front) or
     `run<runNum>_strip<strip>_B.png` (back).

---

## 8. Running

module load root
root -l DSSD_AnalyseSpectra_v2.C

or
From ROOT:

```bash
root -l
.L DSSD_AnalyseSpectra_v2.C+
DSSD_AnalyseSpectra_v2();
```

Edit the **USER INPUT** block at the top to match your run number, file paths, limits, and preferred fit mode before running.
