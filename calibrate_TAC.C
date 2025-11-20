//Calibrator for TAC using time calibrator signal. 
//Uses the sorted root file 
//Plots the TAC histogram and finds the position of each peak
//Sorts the peaks along the x-axis (TSpectrum doesn't by itself)
//Plots the peak channel as function of time
//Does a linear fit and gives calibraiton slope and offset 
void calibrate_TAC()
{   
  //Variable declaration
    int runNum = 2152; // Select a run
    //Run input path
    string inputDir  = "/mnt/analysis/e20008/rootfiles";
    string inputPrefix  = "run"; // If there are zeros before run number add them here
    char runfileName[100]; 
    sprintf(runfileName, "%s/%s%d-00.root",inputDir.c_str(),inputPrefix.c_str(),runNum);
    // TAC histogram 
    int bins = 64000;
    int min = 0; 
    int max = 64000;
    int peakNum = 20; //How many peaks are roughly in the hist (doesn't have to be exact)
    char hCommand[100], gate[200], spectitle[100];
    char name[100];    
 
    TFile *f = new TFile(runfileName);
    TTree *tin = (TTree*)f->Get("t");

    //Canvas settings
    TCanvas *c1 = new TCanvas("c1","c1",1000,700);
    c1->cd();
    c1->ToggleEventStatus();
    c1->SetGrid();
    
    //Set the histogram to be filled, the gate and the histogram name
    sprintf(hCommand,"energy_TAC>>h");
    sprintf(gate,"energy_TAC>0"); 
    sprintf(spectitle, "TAC time");
    
    //Histogram initialization
    TH1D *h = new TH1D("h","TAC [Channels]",bins,min,max);
    tin->Draw(hCommand,gate);
   
    //Find peaks
    TSpectrum ts(peakNum);
    int peakFound = ts.Search(h,2.0,"",1000);
    auto peakChannel = ts.GetPositionX();
    vector<int> peakChannel_sorted(peakChannel, peakChannel+peakFound);
    sort(peakChannel_sorted.begin(), peakChannel_sorted.end());
    
    //Canvas settings
    TCanvas *c2 = new TCanvas("c2","c2",1000,700);
    c2->cd();
    c2->ToggleEventStatus();
    c2->SetGrid();
    
    //Calibration graph
    TGraph *gr = new TGraph();
    int j = 40; //40ns time calibrator
    cout << "\n" << "Using a " << j << " ns calibrator." << endl; 
    for ( int i=0; i<peakFound; i++)
    {   
	gr->SetPoint(i, peakChannel_sorted[i], j);
        j= j+40;
     }
    
    TF1 *p = new TF1("p","pol1",min,max);
    gr->SetTitle("TAC calibration");
    gr->SetMarkerColor(4);
    gr->SetMarkerStyle(21);
    gr->Fit("pol1","F");
    gr->Draw("AP");
    gr->GetXaxis()->SetTitle("Channel");
    gr->GetYaxis()->SetTitle("Time (ns)");
    
    c2->Modified();
    c2->Update();
    gSystem->ProcessEvents();
   
    double offset = p->GetParameter(0);
    double offset_err = p->GetParError(0);
    double slope = p->GetParameter(1);
    double slope_err = p->GetParError(1);
    
  
    //Save a picture and a root file of the histo
    //c1->SaveAs("TAC_hist.png");
    //c2->SaveAs("TAC_calibration.png");
    cout << "\n" << "slope = "<< slope << "\t" << "offset = " << offset << endl;
    
}   
