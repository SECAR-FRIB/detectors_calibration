#include <fstream>
#include <iostream>
#include <limits>
void calibrate_DSSD()
{
    // *** Calibration Runs ***
    int totalRuns = 2; //total number of runs to be used in the calibration
    int totalChannels = 64; //total DSSD channels
    int runNum[] = {2006, 2035}; //  the run numbers for the calibration
    Double_t energy[] = {2571, 5065}; // calculated energy of every run
    double ey[] = {200,200}; //calculated uncertainty of the energy 
    auto DSSD_map_file = "DSSD_map_2024.dat";
    auto DSSD_calibrate_output = "DSSD_calibration.dat";

    //Variable declaration
    char fileName[100], title[100], saveName[100], out[100];
    ofstream fileout;
    fileout.open(DSSD_calibrate_output);  
    
    // *** Data input ***
    // Open the DSSD map file for assigning strips and sides
    int mod[totalChannels], chan[totalChannels], side[totalChannels], strip[totalChannels];
    ifstream DSSDMap(DSSD_map_file); 

    //Open run every output and read peak position and uncertainty per strip
    int mod_o[totalRuns][totalChannels], chan_o[totalRuns][totalChannels];
    Double_t peak[totalRuns][totalChannels], sigma[totalRuns][totalChannels], resolution[totalRuns][totalChannels];

    for(int j=0; j<totalRuns; j++)
    {   
        sprintf(out,"output_run%d_DSSD.txt",runNum[j]);
        ifstream output_DSSD(out);
        for(int i=0; i<totalChannels; i++)
        {   
            DSSDMap >> mod[i] >> chan[i] >> strip[i] >> side[i]; 
            output_DSSD >> mod_o[j][i] >> chan_o[j][i] >> peak[j][i] >> sigma[j][i]; 
        }
    }
        
    // *** Calibration ***
    //Loop over every strip and plot the peak position per run vs the calculated energy
    for(int i=0; i<64; i++)
    {
        // *** Graph initialization ***
        TCanvas *c1 = new TCanvas("c1","c1",1000,700);
        c1->cd();
        c1->ToggleEventStatus();
        c1->SetGrid();
        TGraphErrors *gr = new TGraphErrors();
        TF1 *p = new TF1("p","pol1",100,5000);
        
        //Read the peak position per run and set it as a plot point vs the energy
        for(int j=0; j<totalRuns; j++)
        { 
            if(peak[j][i]>1)
            {   
                gr->SetPoint(j,peak[j][i],energy[j]);
                gr->SetPointError(j,sigma[j][i],ey[j]);
            }
            else {
                gr->SetPoint(j,0,0);
                gr->SetPointError(j,1,1);
            }
        }
        // *** Graph settings ***
        //Set the graph title as strip number and side
        if(mod[i]==1|mod[i]==2)
            sprintf(title,"Strip %d Front",strip[i]);
        else
            sprintf(title,"Strip %d Back",strip[i]);

        gr->SetTitle(title);
        gr->SetMarkerColor(kBlack);
        gr->SetMarkerStyle(21);
        gr->GetXaxis()->SetTitle("channel");
        gr->GetYaxis()->SetTitle("energy (MeV)");
        //gr->GetXaxis()->SetRangeUser(10000,20000);
        //gr->GetYaxis()->SetRangeUser(120,160);
        
        //Fit a line through the points
        gr->Draw("AP"); //Plot axis and points
        //gr->Fit("p","W"); //Set all weights to 1 for non empty bins; ignore error bars
        gr->Fit("p","F"); //Uses the default minimizer (e.g. Minuit) when fitting a linear function (e.g. polN) instead of the linear fitter.
        p->SetLineColor(kMagenta);
        p->SetLineStyle(kDashed);

        Double_t offset = p->GetParameter(0);
        Double_t offset_err = p->GetParError(0);
        Double_t gain = p->GetParameter(1);
        Double_t gain_err = p->GetParError(1);
        
        c1->Modified(); 
        c1->Update();
        gSystem->ProcessEvents();
        //c1->WaitPrimitive();

        //Save a picture and/or a root file of the graph
        if(mod[i]==1|mod[i]==2) sprintf(saveName,"plots/F_Strip%d.png",strip[i]);
        else sprintf(saveName,"plots/B_Strip%d.png",strip[i]);

        if(gain!=0.0)
        {
            c1->SaveAs(saveName);
            fileout << side[i] << "\t" << strip[i] << "\t" << gain << "\t" << offset << "\t" << gain_err << "\t" << offset_err << endl;
        }
        else fileout << side[i] << "\t" << strip[i] << "\t" << 1.0 << "\t" << 0.0 << "\t" << 0.0 << "\t" << 0.0 << endl;

        c1->Clear();
    }
    fileout.close();
    
    cout << "\n" << "You're all done. Bye!" << endl;

}
