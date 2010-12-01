/* Xfibres Diffusion Partial Volume Model  

    Tim Behrens, Saad Jbabdi  - FMRIB Image Analysis Group
 
    Copyright (C) 2005 University of Oxford  */

/*  CCOPYRIGHT  */

#include <iostream>
#include <fstream>
#include <iomanip>
#define WANT_STREAM
#define WANT_MATH
#include <string>
#include <math.h>
#include "utils/log.h"
#include "utils/tracer_plus.h"
#include "miscmaths/miscprob.h"
#include "miscmaths/miscmaths.h"
#include "miscmaths/nonlin.h"
#include "newimage/newimageall.h"
#include "stdlib.h"
#include "fibre.h"
#include "xfibresoptions.h"
#include "diffmodels.h"

using namespace FIBRE;
using namespace Xfibres;
using namespace Utilities;
using namespace NEWMAT;
using namespace NEWIMAGE;
using namespace MISCMATHS;


////////////////////////////////////////////////
//       Some USEFUL FUNCTIONS
////////////////////////////////////////////////


Matrix form_Amat(const Matrix& r,const Matrix& b)
{
  Matrix A(r.Ncols(),7);
  Matrix tmpvec(3,1), tmpmat;
  
  for( int i = 1; i <= r.Ncols(); i++){
    tmpvec << r(1,i) << r(2,i) << r(3,i);
    tmpmat = tmpvec*tmpvec.t()*b(1,i);
    A(i,1) = tmpmat(1,1);
    A(i,2) = 2*tmpmat(1,2);
    A(i,3) = 2*tmpmat(1,3);
    A(i,4) = tmpmat(2,2);
    A(i,5) = 2*tmpmat(2,3);
    A(i,6) = tmpmat(3,3);
    A(i,7) = 1;
  }
  return A;
}

inline SymmetricMatrix vec2tens(ColumnVector& Vec){
  SymmetricMatrix tens(3);
  tens(1,1)=Vec(1);
  tens(2,1)=Vec(2);
  tens(3,1)=Vec(3);
  tens(2,2)=Vec(4);
  tens(3,2)=Vec(5);
  tens(3,3)=Vec(6);
  return tens;
}



////////////////////////////////////////////
//       MCMC SAMPLE STORAGE
////////////////////////////////////////////


class Samples{
  xfibresOptions& opts;
  Matrix m_dsamples;
  Matrix m_d_stdsamples;
  Matrix m_S0samples;
  Matrix m_f0samples;
  Matrix m_lik_energy;

//   // storing signal
//   Matrix m_mean_sig;
//   Matrix m_std_sig;
//   Matrix m_sig2;

  vector<Matrix> m_thsamples;
  vector<Matrix> m_phsamples;
  vector<Matrix> m_fsamples;
  vector<Matrix> m_lamsamples;

  //for storing means
  RowVector m_mean_dsamples;
  RowVector m_mean_d_stdsamples;
  RowVector m_mean_S0samples;
  RowVector m_mean_f0samples;
  RowVector m_mean_tausamples;
  vector<Matrix> m_dyadic_vectors;
  vector<RowVector> m_mean_fsamples;
  vector<RowVector> m_mean_lamsamples;

  float m_sum_d;
  float m_sum_d_std;
  float m_sum_S0;
  float m_sum_f0;
  float m_sum_tau;
  vector<SymmetricMatrix> m_dyad;
  vector<float> m_sum_f;
  vector<float> m_sum_lam;
  int m_nsamps;
  ColumnVector m_vec;
  
  volume<int> m_vol2matrixkey;
  Matrix m_matrix2volkey;
  volume<int> m_beenhere;

  
public:

  Samples( volume<int> vol2matrixkey,Matrix matrix2volkey,int nvoxels,int nmeasures):
    opts(xfibresOptions::getInstance()),m_vol2matrixkey(vol2matrixkey),m_matrix2volkey(matrix2volkey){
    
    m_beenhere=m_vol2matrixkey*0;
    int count=0;
    int nsamples=0;
    
    for(int i=0;i<opts.njumps.value();i++){
      count++;
      if(count==opts.sampleevery.value()){
	count=0;nsamples++;
      }
    }
 

    m_dsamples.ReSize(nsamples,nvoxels);
    m_dsamples=0;
    m_S0samples.ReSize(nsamples,nvoxels);
    m_S0samples=0;
    m_lik_energy.ReSize(nsamples,nvoxels);
    
    // m_mean_sig.ReSize(nmeasures,nvoxels);
//     m_mean_sig=0;
//     m_std_sig.ReSize(nmeasures,nvoxels);
//     m_std_sig=0;
//     m_sig2.ReSize(nmeasures,nvoxels);
//     m_sig2=0;

    m_mean_dsamples.ReSize(nvoxels);
    m_mean_dsamples=0;
    m_mean_S0samples.ReSize(nvoxels);
    m_mean_S0samples=0;
    Matrix tmpvecs(3,nvoxels);
    tmpvecs=0;
    m_sum_d=0;
    m_sum_S0=0;

    if(opts.modelnum.value()==2){
      m_d_stdsamples.ReSize(nsamples,nvoxels);
      m_d_stdsamples=0;
      m_mean_d_stdsamples.ReSize(nvoxels);
      m_mean_d_stdsamples=0;
      m_sum_d_std=0;
    }

    if (opts.f0.value()){
      m_f0samples.ReSize(nsamples,nvoxels);
      m_f0samples=0;
      m_mean_f0samples.ReSize(nvoxels);
      m_mean_f0samples=0;
      m_sum_f0=0;
    }

  if (opts.rician.value()){
      m_mean_tausamples.ReSize(nvoxels);
      m_mean_tausamples=0;
      m_sum_tau=0;
    }


    SymmetricMatrix tmpdyad(3);
    tmpdyad=0;
    m_nsamps=nsamples;
    m_vec.ReSize(3);
    for(int f=0;f<opts.nfibres.value();f++){
      m_thsamples.push_back(m_S0samples);
      m_phsamples.push_back(m_S0samples);
      m_fsamples.push_back(m_S0samples);
      m_lamsamples.push_back(m_S0samples);
      

      m_dyadic_vectors.push_back(tmpvecs);
      m_mean_fsamples.push_back(m_mean_S0samples);
      m_mean_lamsamples.push_back(m_mean_S0samples);

      m_sum_lam.push_back(0);
      m_sum_f.push_back(0);
      m_dyad.push_back(tmpdyad);
    }
 
  }
  
  
  void record(Multifibre& mfib, int vox, int samp){
    m_dsamples(samp,vox)=mfib.get_d();
    m_sum_d+=mfib.get_d();
    if(opts.modelnum.value()==2){
      m_d_stdsamples(samp,vox)=mfib.get_d_std();
      m_sum_d_std+=mfib.get_d_std();
    }
    if (opts.f0.value()){
      m_f0samples(samp,vox)=mfib.get_f0();
      m_sum_f0+=mfib.get_f0();
    }
    if (opts.rician.value())
      m_sum_tau+=mfib.get_tau();
    
    m_S0samples(samp,vox)=mfib.get_S0();
    m_sum_S0+=mfib.get_S0();
    m_lik_energy(samp,vox)=mfib.get_likelihood_energy();
    for(int f=0;f<opts.nfibres.value();f++){
      float th=mfib.fibres()[f].get_th();
      float ph=mfib.fibres()[f].get_ph();
      m_thsamples[f](samp,vox)=th;
      m_phsamples[f](samp,vox)=ph;
      m_fsamples[f](samp,vox)=mfib.fibres()[f].get_f();
      m_lamsamples[f](samp,vox)=mfib.fibres()[f].get_lam();
      //for means
      m_vec << sin(th)*cos(ph) << sin(th)*sin(ph)<<cos(th) ;
      m_dyad[f] << m_dyad[f]+m_vec*m_vec.t();
      m_sum_f[f]+=mfib.fibres()[f].get_f();
      m_sum_lam[f]+=mfib.fibres()[f].get_lam();
    }

//     for(int i=1;i<=m_sig2.Nrows();i++){
//       float sig=mfib.get_signal()(i);
//       m_mean_sig(i,vox)+=sig;
//       m_sig2(i,vox)+=(sig*sig);
//     }
  }
  
  void finish_voxel(int vox){
    m_mean_dsamples(vox)=m_sum_d/m_nsamps;
    if(opts.modelnum.value()==2)
      m_mean_d_stdsamples(vox)=m_sum_d_std/m_nsamps;
    if(opts.f0.value())
      m_mean_f0samples(vox)=m_sum_f0/m_nsamps;
    if(opts.rician.value())
      m_mean_tausamples(vox)=m_sum_tau/m_nsamps;

    m_mean_S0samples(vox)=m_sum_S0/m_nsamps;
    m_sum_d=0;
    m_sum_S0=0;
    m_sum_tau=0;
    if(opts.modelnum.value()==2)
      m_sum_d_std=0;
    if (opts.f0.value())
      m_sum_f0=0;

    DiagonalMatrix dyad_D; //eigenvalues
    Matrix dyad_V; //eigenvectors
    int nfibs=0;
    for(int f=0;f<opts.nfibres.value();f++){
      
      EigenValues(m_dyad[f],dyad_D,dyad_V);
      int maxeig;
      if(dyad_D(1)>dyad_D(2)){
	if(dyad_D(1)>dyad_D(3)) maxeig=1;
	else maxeig=3;
      }
      else{
	if(dyad_D(2)>dyad_D(3)) maxeig=2;
	else maxeig=3;
      }
      m_dyadic_vectors[f](1,vox)=dyad_V(1,maxeig);
      m_dyadic_vectors[f](2,vox)=dyad_V(2,maxeig);
      m_dyadic_vectors[f](3,vox)=dyad_V(3,maxeig);
      
      if((m_sum_f[f]/m_nsamps)>0.05){
	nfibs++;
      }
      m_mean_fsamples[f](vox)=m_sum_f[f]/m_nsamps;
      m_mean_lamsamples[f](vox)=m_sum_lam[f]/m_nsamps;
      
      m_dyad[f]=0;
      m_sum_f[f]=0;
      m_sum_lam[f]=0;
    }
    cout<<m_mean_f0samples(vox)<<" "<<m_mean_dsamples(vox)<<endl;

    m_beenhere(int(m_matrix2volkey(vox,1)),int(m_matrix2volkey(vox,2)),int(m_matrix2volkey(vox,3)))=nfibs;
  }
  
  
  bool neighbour_initialise(int vox, Multifibre& mfibre){
    int xx = int(m_matrix2volkey(vox,1));
    int yy = int(m_matrix2volkey(vox,2));
    int zz = int(m_matrix2volkey(vox,3));
    int voxn=1,voxbest=1;
    bool ret=false;
    int maxfib=1;
    float maxsf=0;
    for(int x=xx-1;x<=xx+1;x++){
      for(int y=yy-1;y<=yy+1;y++){
	for(int z=zz-1;z<=zz+1;z++){
	  if(m_beenhere(x,y,z)>=maxfib){
	    float sumf=0;
	    voxn=m_vol2matrixkey(x,y,z);
	    for(unsigned int fib=0;fib<m_mean_fsamples.size();fib++){
	      if(voxn!=0)
		sumf+=m_mean_fsamples[fib](voxn);
	      else sumf=0;
	     	    }
	    if(sumf>maxsf){
	      maxsf=sumf;
	      maxfib=m_beenhere(x,y,z);
	      voxbest=voxn;
	      
	      ret=true;
	    }
	  }
	
	  
	}
      } 
    }
    ret=(maxfib>1); //
    if(ret){
      mfibre.set_d(m_mean_dsamples(voxbest));
      mfibre.set_S0(m_mean_S0samples(voxbest));
      for(int f=0; f<opts.nfibres.value(); f++){
	
	float th;
	float ph;
	ColumnVector vec(3);
	vec(1)= m_dyadic_vectors[f](1,voxbest);
	vec(2)= m_dyadic_vectors[f](2,voxbest);
	vec(3)= m_dyadic_vectors[f](3,voxbest);
	cart2sph(vec,th,ph);
	if(f==0)
	  // mfibre.addfibre(th,ph,m_mean_fsamples[f](voxbest),1,false);//no a.r.d. on first fibre
	  mfibre.addfibre(th,ph,m_mean_fsamples[f](voxbest),opts.all_ard.value());//is all_ard, then turn ard on here
	else
	  mfibre.addfibre(th,ph,m_mean_fsamples[f](voxbest),true);

      }
	
      
    }
    return ret;
    
  }
  
  
  void save(const volume<float>& mask){
    volume4D<float> tmp;
    //So that I can sort the output fibres into
    // files ordered by fibre fractional volume..
    vector<Matrix> thsamples_out=m_thsamples;
    vector<Matrix> phsamples_out=m_phsamples;
    vector<Matrix> fsamples_out=m_fsamples;
    vector<Matrix> lamsamples_out=m_lamsamples;
    
    vector<Matrix> dyadic_vectors_out=m_dyadic_vectors;
    vector<Matrix> mean_fsamples_out;
    for(unsigned int f=0;f<m_mean_fsamples.size();f++)
      mean_fsamples_out.push_back(m_mean_fsamples[f]);

    Log& logger = LogSingleton::getInstance();
    if(opts.modelnum.value()==1){
      tmp.setmatrix(m_mean_dsamples,mask);
      save_volume4D(tmp,logger.appendDir("mean_dsamples"));
    }
    else if(opts.modelnum.value()==2){
      tmp.setmatrix(m_mean_dsamples,mask);
      save_volume4D(tmp,logger.appendDir("mean_dsamples"));
      tmp.setmatrix(m_mean_d_stdsamples,mask);
      save_volume4D(tmp,logger.appendDir("mean_d_stdsamples"));
      
      tmp.setmatrix(m_dsamples,mask);
      save_volume4D(tmp,logger.appendDir("dsamples"));
      tmp.setmatrix(m_d_stdsamples,mask);
      save_volume4D(tmp,logger.appendDir("d_stdsamples"));
    }
    if (opts.f0.value()){
      tmp.setmatrix(m_mean_f0samples,mask);
      save_volume4D(tmp,logger.appendDir("mean_f0samples"));
      tmp.setmatrix(m_f0samples,mask);
      save_volume4D(tmp,logger.appendDir("f0samples"));
    }
    if (opts.rician.value()){
      tmp.setmatrix(m_mean_tausamples,mask);
      save_volume4D(tmp,logger.appendDir("mean_tausamples"));
    }

    tmp.setmatrix(m_mean_S0samples,mask);
    save_volume4D(tmp,logger.appendDir("mean_S0samples"));
    //tmp.setmatrix(m_lik_energy,mask);
    //save_volume4D(tmp,logger.appendDir("lik_energy"));

    //Sort the output based on mean_fsamples
    // 
    vector<Matrix> sumf;
    for(int f=0;f<opts.nfibres.value();f++){
      Matrix tmp=sum(m_fsamples[f],1);
      sumf.push_back(tmp);
    }  
    for(int vox=1;vox<=m_dsamples.Ncols();vox++){
      vector<pair<float,int> > sfs;
      pair<float,int> ftmp;
      
      for(int f=0;f<opts.nfibres.value();f++){
	ftmp.first=sumf[f](1,vox);
	ftmp.second=f;
	sfs.push_back(ftmp);
      }
      sort(sfs.begin(),sfs.end());
      
      for(int samp=1;samp<=m_dsamples.Nrows();samp++){
	for(int f=0;f<opts.nfibres.value();f++){;
	  thsamples_out[f](samp,vox)=m_thsamples[sfs[(sfs.size()-1)-f].second](samp,vox);
	  phsamples_out[f](samp,vox)=m_phsamples[sfs[(sfs.size()-1)-f].second](samp,vox);
	  fsamples_out[f](samp,vox)=m_fsamples[sfs[(sfs.size()-1)-f].second](samp,vox);
	  lamsamples_out[f](samp,vox)=m_lamsamples[sfs[(sfs.size()-1)-f].second](samp,vox);
	}
      }
      
      for(int f=0;f<opts.nfibres.value();f++){
	mean_fsamples_out[f](1,vox)=m_mean_fsamples[sfs[(sfs.size()-1)-f].second](vox);
	dyadic_vectors_out[f](1,vox)=m_dyadic_vectors[sfs[(sfs.size()-1)-f].second](1,vox);
	dyadic_vectors_out[f](2,vox)=m_dyadic_vectors[sfs[(sfs.size()-1)-f].second](2,vox);
	dyadic_vectors_out[f](3,vox)=m_dyadic_vectors[sfs[(sfs.size()-1)-f].second](3,vox);
      }
      
    }
    // save the sorted fibres
    for(int f=0;f<opts.nfibres.value();f++){
      //      element_mod_n(thsamples_out[f],M_PI);
      //      element_mod_n(phsamples_out[f],2*M_PI);
      tmp.setmatrix(thsamples_out[f],mask);
      string oname="th"+num2str(f+1)+"samples";
      save_volume4D(tmp,logger.appendDir(oname));
      tmp.setmatrix(phsamples_out[f],mask);
      oname="ph"+num2str(f+1)+"samples";
      save_volume4D(tmp,logger.appendDir(oname));
      tmp.setmatrix(fsamples_out[f],mask);
      oname="f"+num2str(f+1)+"samples";
      save_volume4D(tmp,logger.appendDir(oname));
      //      tmp.setmatrix(lamsamples_out[f],mask);
      //      oname="lam"+num2str(f+1)+"samples";
      //      save_volume4D(tmp,logger.appendDir(oname));
      tmp.setmatrix(mean_fsamples_out[f],mask);
      oname="mean_f"+num2str(f+1)+"samples";
      save_volume(tmp[0],logger.appendDir(oname));
      tmp.setmatrix(dyadic_vectors_out[f],mask);
      oname="dyads"+num2str(f+1);
      save_volume4D(tmp,logger.appendDir(oname));
    }
  }
  
};


////////////////////////////////////////////
//       MCMC HANDLING
////////////////////////////////////////////



class xfibresVoxelManager{
 
  xfibresOptions& opts;
  
  Samples& m_samples;
  int m_voxelnumber;
  const ColumnVector m_data;
  const ColumnVector& m_alpha;
  const ColumnVector& m_beta;
  const Matrix& m_bvecs;
  const Matrix& m_bvals; 
  Multifibre m_multifibre;
public:
  xfibresVoxelManager(const ColumnVector& data,const ColumnVector& alpha, 
		      const ColumnVector& beta, const Matrix& r,const Matrix& b,
		      Samples& samples,int voxelnumber):
    opts(xfibresOptions::getInstance()), 
    m_samples(samples),m_voxelnumber(voxelnumber),m_data(data), 
    m_alpha(alpha), m_beta(beta), m_bvecs(r), m_bvals(b), 
    m_multifibre(m_data,m_alpha,m_beta,m_bvals,opts.nfibres.value(),opts.fudge.value(),opts.modelnum.value(),opts.rician.value(),opts.f0.value(),opts.ardf0.value()){ }
  
   
  void initialise(const Matrix& Amat){
    if (opts.rician.value()){  //For Rician noise model, always use a non-linear initialization
	 ColumnVector res=initialise_nonlin();  //Initialize tau using the variance of the residuals
	 float variance=var(res).AsScalar();
	 float tau=1.0/variance;
      //if (tau<0.01)
      //tau=1.0/(0.429*variance);  //We are at very low signal levels, at the Rayleigh regime, convert sigma_Rician=0.655*sigma_Gaussian??
	 m_multifibre.set_tau(tau);
    }	
    else{                     //For Gaussian noise model
      if(opts.nonlin.value())
	initialise_nonlin();
      else{
	if(!opts.localinit.value())
	  if(!m_samples.neighbour_initialise(m_voxelnumber,m_multifibre))
	    initialise_tensor(Amat);
	  else
	    initialise_tensor(Amat);
      }
    }
    m_multifibre.initialise_energies();
    m_multifibre.initialise_props();
  }
  
  
  void initialise_tensor(const Matrix& Amat){
    DTI dti(m_data,Amat);
    dti.linfit();
    
    float D = dti.get_md();
    if(opts.modelnum.value()==1){
      if(D<=0) D=2e-3;      
      m_multifibre.set_d(D);
    }
    if(opts.modelnum.value()==2){
      D=D*2; //Will significantly underestimate D using mono-exponential tensor model, so initialise with 2*D;
      if(D<=0) D=2e-3;
      m_multifibre.set_d_std(D);//initialise with assumption that std=mean. 
      m_multifibre.set_d(D);
    }
    m_multifibre.set_S0(dti.get_s0());

    float th,ph,f;
    cart2sph(dti.get_v1(),th,ph);
    f = dti.get_fa();
    if(opts.nfibres.value()>0){
      //      m_multifibre.addfibre(th,ph,f,false);//no a.r.d. on first fibre
      m_multifibre.addfibre(th,ph,f,opts.all_ard.value());//if all_ard, then turn ard on here (SJ)
      for(int i=2; i<=opts.nfibres.value(); i++){
	 m_multifibre.addfibre();
      }
    }
  }

 
  //Perform non-linear model fitting and returns a vector with the residuals
  ReturnMatrix initialise_nonlin(){
    ColumnVector residuals(m_data.Nrows()),predicted_signal(m_data.Nrows());

    // where using mono-exponential model
    if(opts.modelnum.value()==1){
      PVM_single pvm(m_data,m_bvecs,m_bvals,opts.nfibres.value(),opts.f0.value());
      pvm.fit(); // this will give th,ph,f in the correct order
      
      float pvmS0, pvmd, pvmf0=0.001;
      ColumnVector pvmf,pvmth,pvmph;
      pvmf  = pvm.get_f();
      pvmth = pvm.get_th();
      pvmph = pvm.get_ph();
      pvmS0 = pvm.get_s0();
      pvmd  = pvm.get_d();
      predicted_signal=pvm.get_prediction();

      if (opts.f0.value()){
	pvmf0=pvm.get_f0();

	//If the full model gives values that are considered implausible, or we are in a CSF voxel (f1<0.05)
	//then fit a model without the f0 and drive f0_init to almost zero 
	if (((opts.nfibres.value()>0) && pvmf(1)<0.05) || pvmd>0.007 || pvmf0>0.4){
	  PVM_single pvm2(m_data,m_bvecs,m_bvals,opts.nfibres.value(),false);
	  pvm2.fit(); // this will give th,ph,f in the correct order
	  pvmf0=0.001;
	  pvmS0=pvm2.get_s0();
	  pvmd=pvm2.get_d();
	  pvmf  = pvm2.get_f();
	  pvmth = pvm2.get_th();
	  pvmph = pvm2.get_ph();
	  predicted_signal=pvm2.get_prediction();
	}
	m_multifibre.set_f0(pvmf0);
	//m_multifibre.set_f0(0.001);
      }
      if(pvmd<0)
	pvmd=2e-3;
   
      m_multifibre.set_S0(pvmS0);
      m_multifibre.set_d(pvmd);
      
      cout<<pvmf0<<" "<<pvmS0<<" "<<pvmd<<" "<<pvmf(1)<<" "<<pvmf(2)<<endl;
   
      if(opts.nfibres.value()>0){
	m_multifibre.addfibre(pvmth(1),
			      pvmph(1),
			      pvmf(1),
			      opts.all_ard.value());//if all_ard, then turn ard on here (SJ)
	for(int i=2; i<=opts.nfibres.value();i++){
	  m_multifibre.addfibre(pvmth(i),
				pvmph(i),
				pvmf(i),
				!opts.no_ard.value());
	}
      }
      residuals=m_data-predicted_signal;
    }
    else{ 
      //////////////////////////////////////////////////////
      // model 2 : non-mono-exponential
      if (opts.f0.value())
	m_multifibre.set_f0(0.001); //Need to include f0 in the non-linear initialization of model2 as well!! For now set it initially to almost zero

      PVM_multi pvm(m_data,m_bvecs,m_bvals,opts.nfibres.value());
      pvm.fit();

      m_multifibre.set_S0(pvm.get_s0());
      if(pvm.get_d()>=0)
	m_multifibre.set_d(pvm.get_d());
      else
	m_multifibre.set_d(2e-3);
      if(pvm.get_d_std()>=0)
	m_multifibre.set_d_std(pvm.get_d());
      else
	m_multifibre.set_d(2e-3);

      ColumnVector pvmf,pvmth,pvmph;
      pvmf  = pvm.get_f();
      pvmth = pvm.get_th();
      pvmph = pvm.get_ph();

      if(opts.nfibres.value()>0){
	m_multifibre.addfibre(pvmth(1),
			      pvmph(1),
			      pvmf(1),
			      opts.all_ard.value());//if all_ard, then turn ard on here (SJ)
	for(int i=2; i<=opts.nfibres.value();i++){
	  m_multifibre.addfibre(pvmth(i),
				pvmph(i),
				pvmf(i),
				!opts.no_ard.value());
	}
	
      }
      residuals=m_data-pvm.get_prediction();
    }
    residuals.Release();
    return residuals;   
  }
  

  /*
  //Initialize the precision tau, if rician noise requested
  void initialise_tau(){  
    vector<float> S0_intensities; float S0avg=0,var=0,sigma;
    for (int i=1; i<=m_data.Nrows(); i++)
      if (m_bvals(1,i)==0){  //Get the S0 intensities
	S0avg+=m_data(i);
	S0_intensities.push_back(m_data(i));
      }
    if (S0_intensities.size()>1){     //If we have many S0s, get the standard deviation of the S0s
      S0avg/=S0_intensities.size();
      for (int i=0; i<(int)S0_intensities.size(); i++)
	var+=(S0_intensities[i]-S0avg)*(S0_intensities[i]-S0avg);
      var/=(S0_intensities.size()-1);
      sigma=sqrt(var);
    }
    else
      sigma=S0_intensities[0]/15.0;  //If we have only one S0, assume that the SNR is 15 and obtain a sigma
    cout<<1.0/sigma/sigma<<endl;
    if (sigma!=0)
      m_multifibre.set_tau((1.0/(sigma*sigma)));
    else
      m_multifibre.set_tau(0.01);
  }  */
  


  void runmcmc(){
    int count=0, recordcount=0,sample=1;//sample will index a newmat matrix 
    for( int i =0;i<opts.nburn.value();i++){
      m_multifibre.jump( !opts.no_ard.value() );
      count++;
      if(count==opts.updateproposalevery.value()){
	m_multifibre.update_proposals();
	count=0;
      }
    }
    
    for( int i =0;i<opts.njumps.value();i++){
      m_multifibre.jump(!opts.no_ard.value());
      count++;
     
      if(opts.verbose.value()) 
	{
	  cout<<endl<<i<<" "<<endl<<endl;
	  m_multifibre.report();
	  
	}
      recordcount++;
      if(recordcount==opts.sampleevery.value()){
	m_samples.record(m_multifibre,m_voxelnumber,sample);
	sample++;
	recordcount=0;
      }
      if(count==opts.updateproposalevery.value()){
	m_multifibre.update_proposals();
	count=0;
	
      }
    }
    
    m_samples.finish_voxel(m_voxelnumber);
  }
    
};




////////////////////////////////////////////
//       MAIN
////////////////////////////////////////////
  
int main(int argc, char *argv[])
{
  try{  

    // Setup logging:
    Log& logger = LogSingleton::getInstance();
    xfibresOptions& opts = xfibresOptions::getInstance();
    opts.parse_command_line(argc,argv,logger);
    srand(xfibresOptions::getInstance().seed.value());
    Matrix datam, bvals,bvecs,matrix2volkey;
    volume<float> mask;
    volume<int> vol2matrixkey;
    bvals=read_ascii_matrix(opts.bvalsfile.value());
    bvecs=read_ascii_matrix(opts.bvecsfile.value());
    if(bvecs.Nrows()>3) bvecs=bvecs.t();
    if(bvals.Nrows()>1) bvals=bvals.t();
    for(int i=1;i<=bvecs.Ncols();i++){
      float tmpsum=sqrt(bvecs(1,i)*bvecs(1,i)+bvecs(2,i)*bvecs(2,i)+bvecs(3,i)*bvecs(3,i));
      if(tmpsum!=0){
	bvecs(1,i)=bvecs(1,i)/tmpsum;
	bvecs(2,i)=bvecs(2,i)/tmpsum;
	bvecs(3,i)=bvecs(3,i)/tmpsum;
      }  
    }
    
    volume4D<float> data;
    read_volume4D(data,opts.datafile.value());
    read_volume(mask,opts.maskfile.value());
    datam=data.matrix(mask); 
    matrix2volkey=data.matrix2volkey(mask);
    vol2matrixkey=data.vol2matrixkey(mask);
 
    Matrix Amat;
    ColumnVector alpha, beta;
    Amat=form_Amat(bvecs,bvals);
    cart2sph(bvecs,alpha,beta);
    Samples samples(vol2matrixkey,matrix2volkey,datam.Ncols(),datam.Nrows());
  
    if(opts.rician.value() && !opts.nonlin.value()) 
      cout<<"Rician noise model requested. Non-linear parameter initialization will be performed, overriding other initialization options!"<<endl;
  
    for(int vox=1;vox<=datam.Ncols();vox++){
      cout <<vox<<"/"<<datam.Ncols()<<endl;
      xfibresVoxelManager  vm(datam.Column(vox),alpha,beta,bvecs,bvals,samples,vox);
      vm.initialise(Amat);
      vm.runmcmc();
    }
    
    samples.save(mask);

  }
  catch(Exception& e) 
    {
      cerr << endl << e.what() << endl;
    }
  catch(X_OptionError& e) 
    {
      cerr << endl << e.what() << endl;
    }

  return 0;
}
