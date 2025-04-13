#include "PMTStyle.h"
#include "PMType.h"

#include "Pedestal.h"
#include "SPEResponse.h"
#include "PMT.h"

#include "DFTmethod.h"

#include "SPEFitter.h"

#include <TApplication.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>

#include <iostream>
using std::cout;
using std::endl;

#include<chrono>
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;


int main(int argc, char ** argv)
{
  if(argc < 3){
    cout << "Please provide filename and hist-name" << endl;
  }
  const char * filename = argv[1];
  const char * histname = argv[2];

  TApplication *myapp=new TApplication("myapp",&argc,argv);

  TFile * fin = TFile::Open(filename);
  TH1D * hist = (TH1D*) fin->Get(histname);

  gROOT->Reset();
  
  PMTStyle::SetDefaultStyle();

  
  TCanvas *c1 = new TCanvas( "c1", "" );
  c1->cd();
  c1->SetLogy();

  Double_t Q0 = 0.0;
  Double_t s0 = 2.0;
  Pedestal ped( Q0, s0 );
  
  Double_t lambda = 1.0/40.0;
  Double_t theta = 8.4;
  Double_t alpha = 1.0/8.0;
  Double_t w = 0.2;
  Double_t p[4] = { lambda, theta, alpha, w };
  SPEResponse gamma( PMType::GAMMA, p );

  Int_t nbins = hist->GetNbinsX();
  Double_t xmin = hist->GetXaxis()->GetBinLowEdge(1);
  Double_t xmax = hist->GetXaxis()->GetBinUpEdge(nbins);
    
  hist->Draw();
  
  high_resolution_clock::time_point start = high_resolution_clock::now();

  SPEFitter fit;

  Double_t mu_test = fit.FindMu( hist, Q0, s0 );
  Double_t g_test = fit.FindG( hist, Q0, mu_test );
  
  Double_t p_test[4] = { 1.0/g_test, 7.0, 1.0/(0.5*g_test), 0.2 };
  
  SPEResponse gamma_test( PMType::GAMMA, p_test );
  DFTmethod dft( 2.0*nbins, xmin, xmax, gamma_test );
    
  dft.wbin = hist->GetBinWidth(1);
  
  dft.Norm = hist->GetEntries();
  
  dft.Q0 = Q0;
  dft.s0 = s0;
  
  dft.mu = mu_test;
  
  
  fit.SetDFTmethod( dft );
  fit.FitwDFTmethod( hist );
  
  dft.Norm = fit.vals[0];
  
  dft.Q0 = fit.vals[1];
  dft.s0 = fit.vals[2];

  dft.mu = fit.vals[3]; 
  
  Double_t p_fit[4] = { fit.vals[4], fit.vals[5], fit.vals[6], fit.vals[7] };
  dft.spef.SetParams( p_fit );
  
  high_resolution_clock::time_point end = high_resolution_clock::now();

  TGraph *grBF = dft.GetGraph();
  grBF->Draw( "SAME,L" );

  TGraph *grPE[25];
    
  for ( Int_t i=0; i<25; i++ )
    {
      grPE[i] = dft.GetGraphN( i );
      grPE[i]->Draw( "SAME,L" );
      
    }
  
  
  Double_t Gtrue = ( w/alpha+(1.0-w)/lambda );
  Double_t Gfit = ( fit.vals[7]/fit.vals[6]+(1.0-fit.vals[7])/fit.vals[4] ); 
  
  
  cout << " True Gain : " << Gtrue << endl;
  cout << " BF Gain   : " << Gfit  << endl;
  cout << " Deviation : " << ( Gfit/Gtrue - 1.0 )*100.0 << endl;
  
  cout << "" << endl;
  cout << "" << endl;
  
  

  
  cout << " ... the macro ends ! " << endl;
	  
  cout << "" << endl;
  
      
  duration<double> dura = duration_cast<duration<double>>(end-start);
    
  cout << " ---> "<< dura.count() << " seconds" << endl;  
    
  cout << "" << endl;

  cout << "" << endl;
    
  myapp->Run();
  return 0;
  
}
