// Helper: find peak position and fit limits
void FindPeakAndLimits(TH1 *h, double fracLeft, double fracRight, double &peakPos, double &xLow, double &xHigh)
{
  if (!h) return;

  int nbins = h->GetNbinsX();
  int binMax = h->GetMaximumBin();
  peakPos = h->GetBinCenter(binMax);

  double maxContent = h->GetBinContent(binMax);
  if (maxContent <= 0) {
    // fallback: whole histogram range
    xLow  = h->GetXaxis()->GetXmin();
    xHigh = h->GetXaxis()->GetXmax();
    return;
  }

  double thrLeft  = fracLeft  * maxContent;
  double thrRight = fracRight * maxContent;

  // walk left
  int binL = binMax;
  while (binL > 1 && h->GetBinContent(binL) > thrLeft) {
    --binL;
  }
  xLow = h->GetBinCenter(binL);

  // walk right
  int binR = binMax;
  while (binR < nbins && h->GetBinContent(binR) > thrRight) {
    ++binR;
  }
  xHigh = h->GetBinCenter(binR);
}


void DSSD_AnalyseSpectra()
{
  int runNum;
  int mod, chan;
  int side[64], strip[64];
  char fileName[100];
  char outFile[100];
  int bins, min, max, bins_new;
  int redoFit, reBin;
  char hCommand[100];
  char gate[200];
  char name[100];
  char specname[100];
  int lim1, lim2, lim3, lim4;
  double peak_1 = -1., peak_2 = -1.;
  double sigma_peak_1 = 0., sigma_peak_2 = 0.;

  // =========================
  // === USER INPUT ===
  runNum = 2035;  // Select a run
  std::string inputDir  = "/mnt/analysis/e20008/rootfiles"; // Input path
  std::string inputPrefix  = "run"; // If there are zeros before run number add them here 
  ifstream DSSDMap("DSSD_map_2024.dat"); // DSSD strip map file

  std::string outputDir = ".";   // Output path // (or "." to save locally)
  std::string outputPrefix = "output_run";
  std::string outputTag    = "DSSD"; 
  bool save_fig = true; 
  std::string plotsDir = "plots"; // directory where PNGs of histos with fits will be saved
  
  // Spectra plotting and fitting input
  min = 1; // Minimum channel on spectrum
  max = 4000; // Maximum channel on spectrum
  bins = (max-min)/40; 
  int min_counts = 100;  // Minimum statistics in spectrum (within min/max) to be considered useful
  // Choose fit mode once for all strips
  int fitMode = 1; // 1 = single Gaussian (auto limits), 2 = double Gaussian (manual)
  bool autoFit = true; // Can only be true for single Gaussian fit
  double leftLim = 0.75; // Fit limit left side of peak
  double rightLim = 0.2; // Fit limit right side of peak
  // === END OF USER INPUT === 
  // =========================


  for(int i=0; i<64; i++)
  {
    DSSDMap >> mod >> chan >> strip[i] >> side[i];
  }  

  if (fitMode != 1 && fitMode != 2) {
    cout << "Unknown choice, defaulting to single Gaussian (1)." << endl;
    fitMode = 1;
  }

  sprintf(fileName, "%s/%s%d-00.root",inputDir.c_str(),inputPrefix.c_str(),runNum);
  sprintf(outFile, "%s/%s%d_%s.txt", outputDir.c_str(), outputPrefix.c_str(), runNum, outputTag.c_str());

  TFile *f = new TFile(fileName);
  TTree *tin = (TTree*)f->Get("t");
  TCanvas *c1 = new TCanvas("c1","c1",1000,700);
  ofstream fileout;
  fileout.open(outFile, ios::app); 

  //Loop over every channel of the DSSD (front & back)
  for(int i=0; i<64; i++)
  {  
    //There are 4 modules with 16 channels each. 
    //Channel numbering starts at 0, module numbering starts at 1
    //Modules 1 & 2 have the front channels, 3 & 4 have the back channels
    //Front channels are vertical stripes, back channels are horizontal 
    cout<< "\n" << "Iteration status: "<<i+1<<"/64"<<endl;
    mod = (i/16)+1;
    chan = i%16;
    
    //Set the histogram to be filled, the gate and the histogram name
    sprintf(hCommand,"energy_Si_raw>>h");
    sprintf(gate,"energy_Si_raw[%d]>0",i); 

    char sideChar;
    const char* sideLabel;

    if (mod == 1 || mod == 2) {
      sideChar  = 'F';
      sideLabel = "Front";
    } else {
      sideChar  = 'B';
      sideLabel = "Back";
    }
    sprintf(name, "%s/run%d_strip%d_%c.png", plotsDir.c_str(), runNum, strip[i], sideChar);
    sprintf(specname, "Strip %d %s - module%d channel%d", strip[i], sideLabel, mod, chan);

    //Canvas settings
    c1->cd();
    c1->ToggleEventStatus();
    gPad->SetGridx();
    gPad->SetGridy();

    //Histogram initialization
    TH1D *h = new TH1D("h","h",bins,min,max);
    tin->Draw(hCommand,gate);

    //Histogram drawing settings
    h->SetTitle(specname);
    h->GetXaxis()->SetTitle("channel");
    h->GetYaxis()->SetTitle("counts per channel");
    
    //Update the canvas to show the plot
    c1->Modified(); 
    c1->Update();
    gSystem->ProcessEvents();
    

    // === BINNING BLOCK ===
    reBin=0; // Skip rebinning option
    // Check if the histogram has enough events 
    double k = h->Integral(h->FindFixBin(min), h->FindFixBin(max),"");
    if(k<min_counts)
    {
      h->Delete();
      fileout << mod << "\t" << chan << "\t" << 0 << "\t" << 0 << "\t" << 0 << endl;
      cout<<"Not enough statistics"<<endl;
      continue;
    }

    // Check if the histogram needs rebinning
    //cin.clear();
    //cout<< "\n" <<"Do you need to rebin? If yes enter 1"<<endl;
    //cin >> reBin;

    if(reBin==1)   
    {
      cin.clear();
      cout<< "\n" <<"At module "<<mod<<" channel "<<chan<<" there are few events."<<endl;
      cout<< "\n" << "Rebin..."<<endl;
      cout<< "Enter bin amount <"<< bins << " : ";
      cin >> bins_new;

      if(bins_new==0) 
      {
        h->Delete();
        fileout << mod << "\t" << chan << "\t" << 0 << "\t" << 0 << "\t" << 0 << "\t" << 0 << endl;
        cout<<"Not valid bins. On to the next!"<<endl;
        continue;
      }

      h->Rebin(bins/bins_new);

      c1->Modified(); 
      c1->Update();
      gSystem->ProcessEvents();
    }
    
    // local mode for this strip, starts as the global default
    int localFitMode = fitMode;

    //Check if the type of the fit is ok or needs to be adjusted
    if(!autoFit)
    {
      cout << "\n"<<"For module " << mod << " channel " << chan << "\n";
      cout << "  - Enter  0  : NO peak / skip this spectrum\n";
      cout << "  - Enter  y  : Fit using current default mode ("
          << (fitMode == 1 ? "single Gaussian" : "double Gaussian") << ")\n";
      cout << "  - Enter  1  : Force SINGLE Gaussian for this spectrum\n";
      cout << "  - Enter  2  : Force DOUBLE Gaussian for this spectrum\n";
      cin >> redoFit;
    }
    else { redoFit=100; }

    //In case you don't want to fit the peak at all 
    if(redoFit==0)
    {
      cout<< "\n" <<"At module "<<mod<<" channel "<<chan<<" there are no events (or you decided to skip). Moving on..."<<endl;
      fileout << mod << "\t" << chan << "\t" << 0 << "\t" << 0 << "\t" << 0 << endl;
      cin.clear();
      h->Delete();
      continue;
    }
    // Decide local fit mode for THIS spectrum
    if (redoFit == 1) {
      localFitMode = 1;  // force single Gaussian
    } else if (redoFit == 2) {
      localFitMode = 2;  // force double Gaussian
    } else {
      redoFit = fitMode; // use default fitMode (already in localFitMode)
    }
      

    // === FITTING BLOCK ===
    h->Draw();
    // reset peaks for this channel
    peak_1 = peak_2 = 0.0;
    sigma_peak_1 = sigma_peak_2 = 0.0;
    Double_t par[9];
    Double_t par_dg[6];

    double peakPos, xLow, xHigh;
    if (localFitMode == 1) {
      // ---------- SINGLE GAUSSIAN, AUTO LIMITS ----------
      FindPeakAndLimits(h, leftLim, rightLim, peakPos, xLow, xHigh);

      cout << "Auto peak position ~ " << peakPos
           << "  |  fit range: [" << xLow << ", " << xHigh << "]" << endl;

      TF1 *g1 = new TF1("g1","gausn",xLow,xHigh);
      h->Fit(g1,"R","",xLow,xHigh);

      peak_1        = g1->GetParameter(1);
      sigma_peak_1  = g1->GetParError(1);

      // second peak stays at 0
      peak_2        = 0.0;
      sigma_peak_2  = 0.0;

      g1->SetLineColor(kRed);
      g1->Draw("same");
    }
    else {
      // ---------- DOUBLE GAUSSIAN, MANUAL LIMITS ----------
      //Set the new parameters for the fit
    
      cout << "\n" << "Enter lower limit: ";
      cin >> lim1;
      cout << "Enter upper limit of the first peak: ";
      cin >> lim2;
      cout << "\n" << "Enter lower limit second peak: ";
      cin >> lim3;
      cout << "Enter upper limit of the second peak: ";
      cin >> lim4;
      
      //First Gaussian
      TF1 *g1 = new TF1("g1","gausn",lim1,lim2);
      h->Fit(g1,"R","",lim1,lim2);
      
      //Second Gaussian
      TF1 *g2 = new TF1("g2","gausn",lim3,lim4);
      h->Fit(g2,"R+","",lim3,lim4);
      
      //Double Gaussian
      TF1 *dg = new TF1("dg","gausn(0)+gausn(3)",lim1,lim4);
      g1->GetParameters(&par[0]);
      g2->GetParameters(&par[3]);
      dg->SetParameters(par); // Set these parameters as initial for the double gaussian fit
      dg->SetLineColor(kRed);      
      h->Fit(dg,"R+","",lim1,lim4); 

      
      g1->SetLineColor(kBlue);
      g2->SetLineColor(kGreen+2);
      g1->Draw("same");
      g2->Draw("same");
      dg->Draw("same");

      //Extract parameters from double Gaussian
      dg->GetParameters(&par_dg[0]);
      peak_1       = dg->GetParameter(1);
      sigma_peak_1 = dg->GetParError(1);
      peak_2       = dg->GetParameter(4);
      sigma_peak_2 = dg->GetParError(4);

      c1->Modified(); 
      c1->Update();
      gSystem->ProcessEvents();
    }
   
    //Optionally save picture
    if (save_fig) {c1->SaveAs(name);}

    // Save fit results
    if (localFitMode == 1) {
      fileout << mod << "\t" << chan << "\t" << peak_1 << "\t" << sigma_peak_1 << endl;}
    else {
      fileout << mod << "\t" << chan << "\t" << peak_1 << "\t" << sigma_peak_1 << "\t" << peak_2 << "\t" << sigma_peak_2 << endl;}
    
    //cout << "Press Enter to continue..." << endl;
    //cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //cin.get();
    if (k > min_counts) {   // for example, only pause when enough counts exist
      gSystem->Sleep(1000);
    }

    //Clean the area
    cin.clear();
    h->Delete();
    c1->Clear();
  }
  c1->Close();
  cout << "\n" << "You're all done. Bye!" << endl;
}
