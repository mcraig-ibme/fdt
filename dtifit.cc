#include <iostream>
#include <cmath>
#include "miscmaths/miscmaths.h"
#include "newmat.h"
#include "dtifitOptions.h"
#include "newimage/newimageall.h"

using namespace std;
using namespace NEWMAT;
using namespace MISCMATHS;
using namespace DTIFIT;
using namespace NEWIMAGE;


const float maxfloat=1e10;
const float minfloat=1e-10;
const float maxlogfloat=23;
const float minlogfloat=-23;
const int maxint=1000000000; 

inline float PI() { return  3.14159265358979;}
inline float min(float a,float b){
  return a<b ? a:b;}
inline float max(float a,float b){
  return a>b ? a:b;}
inline Matrix Anis()
{ 
  Matrix A(3,3);
  A << 1 << 0 << 0
    << 0 << 0 << 0
    << 0 << 0 << 0;
  return A;
}

inline Matrix Is()
{ 
  Matrix I(3,3);
  I << 1 << 0 << 0
    << 0 << 1 << 0
    << 0 << 0 << 1;
  return I;
}

inline ColumnVector Cross(const ColumnVector& A,const ColumnVector& B)
{
  ColumnVector res(3);
  res << A(2)*B(3)-A(3)*B(2)
      << A(3)*B(1)-A(1)*B(3)
      << A(1)*B(2)-B(1)*A(2);
  return res;
}

inline Matrix Cross(const Matrix& A,const Matrix& B)
{
  Matrix res(3,1);
  res << A(2,1)*B(3,1)-A(3,1)*B(2,1)
      << A(3,1)*B(1,1)-A(1,1)*B(3,1)
      << A(1,1)*B(2,1)-B(1,1)*A(2,1);
  return res;
}

float mod(float a, float b){
  while(a>b){a=a-b;}
  while(a<0){a=a+b;}
  return a;
}



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


void tensorfit(DiagonalMatrix& Dd,ColumnVector& evec1,ColumnVector& evec2,ColumnVector& evec3,float& f,float& s0,const Matrix& Amat,const ColumnVector& S)
{
  //Initialise the parameters using traditional DTI analysis
  ColumnVector logS(S.Nrows()),Dvec(7);
  SymmetricMatrix tens;   //Basser's Diffusion Tensor;
  //  DiagonalMatrix Dd;   //eigenvalues
  Matrix Vd;   //eigenvectors
  DiagonalMatrix Ddsorted(3);
  float mDd, fsquared;

  for ( int i = 1; i <= S.Nrows(); i++)
    {
      if(S(i)>0){
	logS(i)=log(S(i));
      }
      else{
	logS(i)=0;
      }
      //      logS(i)=(S(i)/S0)>0.01 ? log(S(i))-log(S0):log(0.01);
    }

  Dvec = -pinv(Amat)*logS;
  if(  Dvec(7) >  -maxlogfloat ){
    s0=exp(-Dvec(7));
  }
  else{
    s0=S.MaximumAbsoluteValue();
  }
  for ( int i = 1; i <= S.Nrows(); i++)
    {
      if(s0<S.Sum()/S.Nrows()){ s0=S.MaximumAbsoluteValue();  }
      logS(i)=(S(i)/s0)>0.01 ? log(S(i)):log(0.01*s0);
    }
  Dvec = -pinv(Amat)*logS;
  s0=exp(-Dvec(7));
  if(s0<S.Sum()/S.Nrows()){ s0=S.Sum()/S.Nrows();  }
  tens = vec2tens(Dvec);
  EigenValues(tens,Dd,Vd);
  mDd = Dd.Sum()/Dd.Nrows();
  int maxind = Dd(1) > Dd(2) ? 1:2;   //finding max,mid and min eigenvalues
  maxind = Dd(maxind) > Dd(3) ? maxind:3;
  int midind;
  if( (Dd(1)>=Dd(2) && Dd(2)>=Dd(3)) || (Dd(1)<=Dd(2) && Dd(2)<=Dd(3)) ){midind=2;}
  else if( (Dd(2)>=Dd(1) && Dd(1)>=Dd(3)) || (Dd(2)<=Dd(1) && Dd(1)<=Dd(3)) ){midind=1;}
  else {midind=3;}
  int minind = Dd(1) < Dd(2) ? 1:2;   //finding maximum eigenvalue
  minind = Dd(minind) < Dd(3) ? minind:3;
  Ddsorted << Dd(maxind) << Dd(midind) << Dd(minind);
  Dd=Ddsorted;
  evec1 << Vd(1,maxind) << Vd(2,maxind) << Vd(3,maxind);
  evec2 << Vd(1,midind) << Vd(2,midind) << Vd(3,midind);
  evec3 << Vd(1,minind) << Vd(2,minind) << Vd(3,minind);

  float numer=(1.5*(Dd(1)-mDd)*(Dd(1)-mDd)+(Dd(2)-mDd)*(Dd(2)-mDd)+(Dd(3)-mDd)*(Dd(3)-mDd));
  float denom=(Dd(1)*Dd(1)+Dd(2)*Dd(2)+Dd(3)*Dd(3));
 
  if(denom>0) fsquared=numer/denom;
  else fsquared=0;
  if(fsquared>0){f=sqrt(fsquared);}
  else{f=0;}
  //  f = sqrt((1.5*(Dd(1)-mDd)*(Dd(1)-mDd)+(Dd(2)-mDd)*(Dd(2)-mDd)+(Dd(3)-mDd)*(Dd(3)-mDd))/(Dd(1)*Dd(1)+Dd(2)*Dd(2)+Dd(3)*Dd(3)));
  // if(f>=0.95) f=0.95;
  //if(f<=0.001) f=0.001;

}

int main(int argc, char** argv)
{
  //parse command line
  dtifitOptions& opts = dtifitOptions::getInstance();
  int success=opts.parse_command_line(argc,argv);
  if(!success) return 0;
   if(opts.verbose.value()){
    cerr<<"data file "<<opts.dtidatafile.value()<<endl;
    cerr<<"mask file "<<opts.maskfile.value()<<endl;
    cerr<<"bvecs     "<<opts.bvecsfile.value()<<endl;
    cerr<<"bvals     "<<opts.bvalsfile.value()<<endl;
    if(opts.littlebit.value()){
      cerr<<"min z     "<<opts.z_min.value()<<endl;
      cerr<<"max z     "<<opts.z_max.value()<<endl;
      cerr<<"min y     "<<opts.y_min.value()<<endl;
      cerr<<"max y     "<<opts.y_max.value()<<endl;
      cerr<<"min x     "<<opts.x_min.value()<<endl;
      cerr<<"max x     "<<opts.x_max.value()<<endl;
    }
  }
  
  // Set random seed:
  Matrix r = read_ascii_matrix(opts.bvecsfile.value());
  Matrix b = read_ascii_matrix(opts.bvalsfile.value());
  volume4D<float> data;
  volume<int> mask;
  volumeinfo tempinfo;
  if(opts.verbose.value()) cerr<<"reading data"<<endl;
  read_volume4D(data,opts.dtidatafile.value(),tempinfo);
  if(opts.verbose.value()) cerr<<"reading mask"<<endl;
  read_volume(mask,opts.maskfile.value());
  if(opts.verbose.value()) cerr<<"ok"<<endl;
  int minx=opts.littlebit.value() ? opts.x_min.value():0;
  int maxx=opts.littlebit.value() ? opts.x_max.value():mask.xsize();
  int miny=opts.littlebit.value() ? opts.y_min.value():0;
  int maxy=opts.littlebit.value() ? opts.y_max.value():mask.ysize();
  int minz=opts.littlebit.value() ? opts.z_min.value():0;
  int maxz=opts.littlebit.value() ? opts.z_max.value():mask.zsize();
  cerr<<minx<<" "<<maxx<<" "<<miny<<" "<<maxy<<" "<<minz<<" "<<maxz<<endl;
  if(opts.verbose.value()) cerr<<"setting up vols"<<endl;
  volume<float> l1(maxx-minx,maxy-miny,maxz-minz);
  volume<float> l2(maxx-minx,maxy-miny,maxz-minz);
  volume<float> l3(maxx-minx,maxy-miny,maxz-minz);
  volume<float> FA(maxx-minx,maxy-miny,maxz-minz);
  volume<float> S0(maxx-minx,maxy-miny,maxz-minz);
  volume4D<float> V1(maxx-minx,maxy-miny,maxz-minz,3);
  volume4D<float> V2(maxx-minx,maxy-miny,maxz-minz,3);
  volume4D<float> V3(maxx-minx,maxy-miny,maxz-minz,3);
  if(opts.verbose.value()) cerr<<"copying input properties to output volumes"<<endl;
  copybasicproperties(data[0],l1);
  copybasicproperties(data[0],l2);
  copybasicproperties(data[0],l3);
  copybasicproperties(data[0],FA);
  copybasicproperties(data[0],S0);
  copybasicproperties(data[0],V1[0]);
  copybasicproperties(data[0],V2[0]);
  copybasicproperties(data[0],V3[0]);
  if(opts.verbose.value()) cerr<<"zeroing output volumes"<<endl;
  l1=0;l2=0;l3=0;FA=0;S0=0;V1=0;V2=0;V3=0;
  if(opts.verbose.value()) cerr<<"ok"<<endl;
  DiagonalMatrix evals(3);
  ColumnVector evec1(3),evec2(3),evec3(3);
  ColumnVector S(data.tsize());
  float fa,s0;
  if(opts.verbose.value()) cerr<<"Forming A matrix"<<endl;
  Matrix Amat = form_Amat(r,b);
  if(opts.verbose.value()) cerr<<"starting the fits"<<endl;
  for(int k = minz; k < maxz; k++){
    cerr<<k<<" slices processed"<<endl;
      for(int j=miny; j < maxy; j++){
	for(int i =minx; i< maxx; i++){
	
	  if(mask(i,j,k)==1){
	    
	    for(int t=0;t < data.tsize();t++){
	      S(t+1)=data(i,j,k,t);
	    }
	   
	    tensorfit(evals,evec1,evec2,evec3,fa,s0,Amat,S);
	    l1(i-minx,j-miny,k-minz)=evals(1);
	    l2(i-minx,j-miny,k-minz)=evals(2);
	    l3(i-minx,j-miny,k-minz)=evals(3);
	    FA(i-minx,j-miny,k-minz)=fa;
	    S0(i-minx,j-miny,k-minz)=s0;
	    V1(i-minx,j-miny,k-minz,0)=evec1(1);
	    V1(i-minx,j-miny,k-minz,1)=evec1(2);
	    V1(i-minx,j-miny,k-minz,2)=evec1(3);
	    V2(i-minx,j-miny,k-minz,0)=evec2(1);
	    V2(i-minx,j-miny,k-minz,1)=evec2(2);
	    V2(i-minx,j-miny,k-minz,2)=evec2(3);
	    V3(i-minx,j-miny,k-minz,0)=evec3(1);
	    V3(i-minx,j-miny,k-minz,1)=evec3(2);
	    V3(i-minx,j-miny,k-minz,2)=evec3(3);
	    




//	    EigenValues(dyad,dyad_D,dyad_V);
	   

	    
//  	    // work out which is the maximum eigenvalue;
//  	    int maxeig;
//  	    if(dyad_D(1)>dyad_D(2)){
//  	      if(dyad_D(1)>dyad_D(3)) maxeig=1;
//  	      else maxeig=3;
//  	    }
//  	    else{
//  	      if(dyad_D(2)>dyad_D(3)) maxeig=2;
//  	      else maxeig=3;
//  	    }
//  	    dyadic_vecs(i-minx,j-miny,k-minz,0)=dyad_V(1,maxeig);
//  	    dyadic_vecs(i-minx,j-miny,k-minz,1)=dyad_V(2,maxeig);
//  	    dyadic_vecs(i-minx,j-miny,k-minz,2)=dyad_V(3,maxeig);
	    



	  }
	}
      }
  }
  
    string fafile=opts.ofile.value()+"_FA";
    string s0file=opts.ofile.value()+"_S0";
    string l1file=opts.ofile.value()+"_L1";
    string l2file=opts.ofile.value()+"_L2";
    string l3file=opts.ofile.value()+"_L3";
    string v1file=opts.ofile.value()+"_V1";
    string v2file=opts.ofile.value()+"_V2";
    string v3file=opts.ofile.value()+"_V3";

    if(opts.littlebit.value()){
      fafile+="littlebit";
      s0file+="littlebit";
      l1file+="littlebit";
      l2file+="littlebit";
      l3file+="littlebit";
      v1file+="littlebit";
      v2file+="littlebit";
      v3file+="littlebit";
    }
  
    save_volume(FA,fafile,tempinfo);
    save_volume(S0,s0file,tempinfo);
    save_volume(l1,l1file,tempinfo);
    save_volume(l2,l2file,tempinfo);
    save_volume(l3,l3file,tempinfo);
    save_volume4D(V1,v1file,tempinfo);
    save_volume4D(V2,v2file,tempinfo);
    save_volume4D(V3,v3file,tempinfo);

  return 0;
}













