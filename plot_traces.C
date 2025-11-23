R__LOAD_LIBRARY(/usr/opt/ddas/6.1-000/lib/libddaschannel.so)
//runs with root6.24/06

#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>
#include <iostream>
#include <vector>


// ------------------------------
// Simple trace-plotting function
// ------------------------------
void plot_traces() {

    // Open ROOT file
    TFile* pFile = new TFile("/mnt/analysis/e20008/dumpedfiles/run2157-00.root");
    if (!pFile || pFile->IsZombie()) {
        std::cerr << "ERROR: Could not open file." << std::endl;
        return;
    }

    // Get tree
    TTree* pTree = nullptr;
    pFile->GetObject("dchan", pTree);
    if (!pTree) {
        std::cerr << "ERROR: Tree 'dchan' not found." << std::endl;
        return;
    }

    // Set up event object
    DDASEvent* pEvent = new DDASEvent();
    pTree->SetBranchAddress("ddasevent", &pEvent);

    // How many traces to draw
    const int maxTracesToPlot = 10;
    int plotCrate = 1;
    int plotMod = 0;
    int plotChan = 8;
    
    TCanvas* c1 = new TCanvas("c1", "Traces", 1500, 1000);
    c1->cd();
    
    int tracesPlotted = 0;

    // Predefined list of colors to cycle through
    int colors[] = { kRed, kBlue, kGreen+2, kMagenta, kOrange+7, kCyan+2 };

    // Loop over entries in the TTree
    for (int i = 0; i < pTree->GetEntries(); ++i) {

        pTree->GetEntry(i);

        std::vector<uint16_t> trace;

        // Loop over channels in this event
        for (int j = 0; j < pEvent->GetNEvents(); j++) {

            ddaschannel* dchan = pEvent->GetData()[j];

            // Select your desired crate/slot/channel
            if (dchan->GetCrateID() == plotCrate &&
                dchan->GetSlotID()  == plotMod+2 &&
                dchan->GetChannelID() == plotChan)
            {
                trace = dchan->GetTrace();
            }
        }

        // Skip if no trace in this event
        if (trace.empty()) continue;

        int size = trace.size();

        // Convert trace index + samples into doubles for TGraph
        std::vector<double> xd(size), yd(size);
        for (int k = 0; k < size; k++) {
            xd[k] = k;
            yd[k] = trace[k];
        }

        // Create graph for this trace
        TGraph* gr = new TGraph(size, &xd[0], &yd[0]);
        int color = colors[tracesPlotted % (sizeof(colors)/sizeof(int))];
        gr->SetLineColor(color);
        gr->SetLineWidth(1);
        gr->SetMarkerStyle(7);

        // Draw first trace normally, the rest "same"
        if (tracesPlotted == 0)
	{
	    // Build the title string
            std::stringstream ss;
            ss <<  maxTracesToPlot << " Traces of"
               << " Mod " << plotMod << " Chan " << plotChan;

            gr->SetTitle(ss.str().c_str());
            gr->Draw("AL");
        }else
            gr->Draw("L SAME");

        tracesPlotted++;

        if (tracesPlotted >= maxTracesToPlot)
            break;
    }

    std::cout << "Plotted " << tracesPlotted << " traces." << std::endl;
}
