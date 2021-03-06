#include "PMTStyle.h"
#include "PMType.h"

#include "Pedestal.h"
#include "SPEResponse.h"
#include "PMT.h"

#include "DFTmethod.h"

#include "SPEFitter.h"

#include <TApplication.h>
#include <TCanvas.h>

#include <iostream>
using std::cout;
using std::endl;

#include<chrono>
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;


int main(int argc, char ** argv)
{
  TApplication *myapp=new TApplication("myapp",&argc,argv);

  
  cout << "" << endl;
  
  cout << " The macro starts ( example3.C ) ... " << endl;

  cout << "" << endl;


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

  Int_t nbins = 250;
  Double_t xmin = -20.0;
  Double_t xmax = 480.0;
    
  PMT specimen( nbins, xmin, xmax, ped, gamma );
  Double_t mu = 1.2;
  Int_t ntot = 2.0e+5;
  specimen.GenSpectrum( ntot, mu );
  specimen.GetSpectrum()->SetStats(0);
  specimen.DrawSpectrum();
  
  
  high_resolution_clock::time_point start = high_resolution_clock::now();

  SPEFitter fit;

  Double_t mu_test = fit.FindMu( specimen.GetSpectrum(), Q0, s0 );
  Double_t g_test = fit.FindG( specimen.GetSpectrum(), Q0, mu_test );
  
  Double_t p_test[4] = { 1.0/g_test, 7.0, 1.0/(0.5*g_test), 0.2 };
  
  SPEResponse gamma_test( PMType::GAMMA, p_test );
  DFTmethod dft( 2.0*nbins, xmin, xmax, gamma_test );
    
  dft.wbin = specimen.GetSpectrum()->GetBinWidth(1);
  
  dft.Norm = ntot;
  
  dft.Q0 = Q0;
  dft.s0 = s0;
  
  dft.mu = mu_test;
  
  
  fit.SetDFTmethod( dft );
  fit.FitwDFTmethod( specimen.GetSpectrum() );
  
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
