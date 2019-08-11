
#include "SPEFitter.h"

using namespace std;

Int_t N;
double xx0[7500];
double yy0[7500];

double wbin0;

DFTmethod dft0;
PMTModel mod0;

double fit_func_fft( const double *x )
{
  double result = 0.0;

  dft0.wbin = wbin0;
  
  double Norm = x[0]; 
  dft0.Norm = Norm;
  
  double Q0 = x[1];
  dft0.Q0 = Q0;
  double s0 = x[2];
  dft0.s0 = s0;
  
  double mu = x[3];
  dft0.mu = mu;
    
  double params0[20];
  for( Int_t i=0; i<dft0.spef.nparams; i++ ) params0[i] = x[i+4];
  dft0.spef.SetParams( params0 );

  dft0.CalculateValues();

  for ( Int_t i=0; i<N; i++ )
    {
      Double_t val = dft0.GetValue( xx0[i] );
      if ( val<1.0e-10 ) val = 1.0e-4;
      
      result += pow( val-yy0[i], 2.0 )/( val );
            
    }
    
  return result;
  
}

double fit_func_mod( const double *x )
{
  double result = 0.0;

  mod0.wbin = wbin0;
      
  double params0[20];
  for( Int_t i=0; i<mod0.nparams; i++ ) params0[i] = x[i];
  mod0.SetParams( params0 );
  
  for ( Int_t i=0; i<N; i++ )
    {
      Double_t val = mod0.GetValue( xx0[i] );
      if ( val<1.0e-10 ) val = 1.0e-4;
      
      result += pow( val-yy0[i], 2.0 )/( val );
            
    }
    
  return result;
  
}

Double_t m_g( Double_t *x, Double_t *par )
{
  Double_t xx = x[0];

  Double_t Norm = par[0];
  
  Double_t Q_0 = par[1];
  Double_t s_0 = par[2];
        
  Double_t arg = 0.0; 
  if ( s_0!=0.0 ) arg = ( xx - Q_0 )/s_0;    
  else cout << "Error: The code tries to divide by zero." << endl;
  
  Double_t result = 1.0/( sqrt( 2 * TMath::Pi() ) * s_0 ) * TMath::Exp( -0.5*arg*arg );
  result *= ( Norm*wbin0 );
    
  return result;
  
}

ClassImp( SPEFitter )


SPEFitter::SPEFitter()
{}

SPEFitter::~SPEFitter()
{}

void SPEFitter::SetDFTmethod( DFTmethod _dft )
{
  dft = _dft;

  dft0 = _dft;
  
}

void SPEFitter::SetPMTModel( PMTModel _mod )
{
  mod = _mod;

  mod0 = _mod;
  
}

Double_t SPEFitter::FindMu( TH1D *hspec, Double_t _Q0, Double_t _s0 )
{
  ped_func = new TF1( "pmt_ped", m_g, _Q0-5.0*_s0, _Q0-5.0*_s0, 3 );
  ped_func->SetParNames( "Norm", "Q", "#sigma" );
  ped_func->SetLineColor( kRed );
  ped_func->SetNpx( 10000 );
  
  wbin0 = hspec->GetXaxis()->GetBinWidth(1);
  
  Double_t Norm = hspec->Integral();
  
  ped_func->SetParameter( 0, Norm );
  ped_func->SetParLimits( 0, Norm*0.01, Norm*100.0 );

  ped_func->SetParameter( 1, _Q0 ); ped_func->SetParLimits( 1, _Q0*0.9, _Q0*1.1 );
  ped_func->SetParameter( 2, _s0 ); ped_func->SetParLimits( 2, _s0*0.9, _s0*1.1 );
  
  
  hspec->Fit( "pmt_ped", "Q0", "", _Q0-3.0*_s0, _Q0+1.8*_s0 );  
  
  Double_t *pars0 = ped_func->GetParameters();
  Double_t _mu = TMath::Log( Norm/pars0[0] );
      
  return _mu;
  
}

Double_t SPEFitter::FindG( TH1D *hspec, Double_t _Q0, Double_t _mu )
{
  Double_t G = ( hspec->GetMean()-_Q0 )/_mu;
  
  return G;
  
}

void SPEFitter::FitwDFTmethod( TH1D *hspec )
{
  N = hspec->GetXaxis()->GetNbins();
  wbin0 = hspec->GetXaxis()->GetBinWidth(1);
  
  for ( Int_t i=0; i<N; i++ )
    {
      xx0[i] = hspec->GetXaxis()->GetBinCenter( i+1 );
      yy0[i] = hspec->GetBinContent( i+1 );
      
    }
  
  mFFT = new ROOT::Minuit2::Minuit2Minimizer();
  
  ROOT::Math::Functor FCA;
  FCA = ROOT::Math::Functor( &fit_func_fft, dft.spef.nparams+4 );
  
  mFFT->SetFunction(FCA);

  mFFT->SetLimitedVariable( 0, "Norm", dft.Norm, dft.Norm*0.001, dft.Norm*0.5, dft.Norm*2.0 );

  mFFT->SetLimitedVariable( 1, "Q0", dft.Q0, dft.Q0*0.01, dft.Q0*0.9, dft.Q0*1.1 );
  mFFT->SetLimitedVariable( 2, "s0", dft.s0, dft.s0*0.01, dft.s0*0.9, dft.s0*1.1 );
  
  mFFT->SetLimitedVariable( 3, "mu", dft.mu, 0.01, dft.mu*0.5, dft.mu*1.5 );
  
  mFFT->SetLimitedVariable( 4, "PAR1", dft.spef.params[0], dft.spef.params[0]*0.001, dft.spef.params[0]*0.1, dft.spef.params[0]*5.0 );
  mFFT->SetLimitedVariable( 5, "PAR2", dft.spef.params[1], dft.spef.params[1]*0.01, dft.spef.params[1]*0.1, dft.spef.params[1]*5.0 );
  mFFT->SetLimitedVariable( 6, "PAR3", dft.spef.params[2], dft.spef.params[2]*0.01, dft.spef.params[2]*0.05, dft.spef.params[2]*50.0 );
  mFFT->SetLimitedVariable( 7, "PAR4", dft.spef.params[3], 0.01, 0.0, 0.5 );
  
  mFFT->SetMaxFunctionCalls(1.E9);
  mFFT->SetMaxIterations(1.E9);
  mFFT->SetTolerance(0.01);
  mFFT->SetStrategy(2);
  mFFT->SetErrorDef(1.0);
  mFFT->Minimize();
  mFFT->Hesse();
  
  
  Int_t ifits = 0;
  while( mFFT->Status()!=0 && ifits<10 )
    { 
      mFFT->Minimize();
      mFFT->Hesse();
      ifits++;
      
    }
  
  if( mFFT->Status()!=0 )
    {
      cout << " * " << endl;
      cout << " * Fit has failed ! " << endl;
      cout << " * " << endl;
      
      return;

    }


  cout << " * " << endl;
  cout << " * Minimization results "  << endl;
  cout << " * " << endl;
  cout << " * No. of calls  : " << mFFT->NCalls() << endl;
  cout << " * Fitter status : " << mFFT->Status() << endl;
  cout << " * " << endl;
  
  Int_t ndim = mFFT->NDim();
  const double *pars = mFFT->X();  
  const double *erpars = mFFT->Errors();
    
  for ( int i=0; i<ndim; i++ )
    {
      cout << " * " << setw(10)  << mFFT->VariableName(i) << " : " << Form( "%.3f", pars[i] ) << " +/- " << Form( "%.3f", erpars[i] ) << endl; 
      cout << " * " << endl;

      vals[i]=pars[i];
      errs[i]=erpars[i];
            
    }
  
  cout << " * " << setw(10) << "chi2/NDOF" << " : " << Form( "%.2f", mFFT->MinValue()/( N-8-1 ) ) << endl;
  cout << " * " << endl;
  
  delete pars;
  delete erpars;

  dft.wbin = wbin0;
    
  dft.Norm = vals[0];
  
  dft.Q0 = vals[1];
  dft.s0 = vals[2];

  dft.mu = vals[3];

  Double_t p[4] = { vals[4], vals[5], vals[6], vals[7] };
  dft.spef.SetParams( p );
  
  cout << "" << endl;
    
  return;
  
}

void SPEFitter::FitwPMTModel( TH1D *hspec )
{
  N = hspec->GetXaxis()->GetNbins();
  wbin0 = hspec->GetXaxis()->GetBinWidth(1);
  
  for ( Int_t i=0; i<N; i++ )
    {
      xx0[i] = hspec->GetXaxis()->GetBinCenter( i+1 );
      yy0[i] = hspec->GetBinContent( i+1 );
      
    }
  
  mMOD = new ROOT::Minuit2::Minuit2Minimizer();
  
  ROOT::Math::Functor FCA;
  FCA = ROOT::Math::Functor( &fit_func_mod, 8 ); 
  
  mMOD->SetFunction(FCA);

  mMOD->SetLimitedVariable( 0, "Norm", mod.params[0], mod.params[0]*0.001, mod.params[0]*0.1, mod.params[0]*10.0 );

  mMOD->SetLimitedVariable( 1, "Q0", mod.params[1], mod.params[1]*0.01, mod.params[1]*0.9, mod.params[1]*1.1 );
  mMOD->SetLimitedVariable( 2, "s0", mod.params[2], mod.params[2]*0.01, mod.params[2]*0.9, mod.params[2]*1.1 );
  
  mMOD->SetLimitedVariable( 3, "mu", mod.params[3], 0.01, mod.params[3]*0.1, mod.params[3]*2.0 );
  
  mMOD->SetLimitedVariable( 4, "PAR1", mod.params[4], mod.params[4]*0.001, mod.params[4]*0.1, mod.params[4]*5.0 );
  mMOD->SetLimitedVariable( 5, "PAR2", mod.params[5], mod.params[5]*0.01, mod.params[5]*0.01, mod.params[5]*100.0 );
  mMOD->SetLimitedVariable( 6, "PAR3", mod.params[6], mod.params[6]*0.01, mod.params[6]*0.01, mod.params[6]*100.0 );
  mMOD->SetLimitedVariable( 7, "PAR4", mod.params[7], 0.01, 0.0, 0.5 );
  
  mMOD->SetMaxFunctionCalls(1.E9);
  mMOD->SetMaxIterations(1.E9);
  mMOD->SetTolerance(0.01);
  mMOD->SetStrategy(2);
  mMOD->SetErrorDef(1.0);
  mMOD->Minimize();
  mMOD->Hesse();
  
  
  Int_t ifits = 0;
  while( mMOD->Status()!=0 && ifits<10 )
    { 
      mMOD->Minimize();
      mMOD->Hesse();
      ifits++;
      
    }
  
  if( mMOD->Status()!=0 )
    {
      cout << " * " << endl;
      cout << " * Fit has failed ! " << endl;
      cout << " * " << endl;
      
      return;

    }

  
  cout << " * " << endl;
  cout << " * Minimization results "  << endl;
  cout << " * " << endl;
  cout << " * No. of calls  : " << mMOD->NCalls() << endl;
  cout << " * Fitter status : " << mMOD->Status() << endl;
  cout << " * " << endl;
  
  Int_t ndim = mMOD->NDim();
  const double *pars = mMOD->X();  
  const double *erpars = mMOD->Errors();
    
  for ( int i=0; i<ndim; i++ )
    {
      cout << " * " << setw(10)  << mMOD->VariableName(i) << " : " << Form( "%.3f", pars[i] ) << " +/- " << Form( "%.3f", erpars[i] ) << endl; 
      cout << " * " << endl;

      vals[i]=pars[i];
      errs[i]=erpars[i];
            
    }
  
  cout << " * " << setw(10) << "chi2/NDOF" << " : " << Form( "%.2f", mMOD->MinValue()/( N-mod.nparams-1 ) ) << endl;
  cout << " * " << endl;
  
  delete pars;
  delete erpars;
  
  Double_t p[8] = { vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7] };
  mod.SetParams( p );
  
  cout << "" << endl;
  
  return;
  
}