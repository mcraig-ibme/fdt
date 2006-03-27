/*  probtrackxOptions.cc

    Tim Behrens, FMRIB Image Analysis Group

    Copyright (C) 2004 University of Oxford  */

/*  CCOPYRIGHT  */

#define WANT_STREAM
#define WANT_MATH

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "probtrackxOptions.h"
#include "utils/options.h"
//#include "newmat.h"
using namespace Utilities;

namespace TRACT {

probtrackxOptions* probtrackxOptions::gopt = NULL;

  void probtrackxOptions::parse_command_line(int argc, char** argv, Log& logger)
  {
    //Do the parsing;
    try{
      for(int a = options.parse_command_line(argc, argv); a < argc; a++) ;
      // setup logger directory
      if( mode.value()=="help"){
	modehelp();
	exit(2);
      }
      else if(help.value() || ! options.check_compulsory_arguments())
	{
	  options.usage();
	  exit(2);
	}   

      else{
	modecheck(); // check all the correct options are set for this mode.
	if(mode.value()!="simple"){  
	  if(forcedir.value())
	    logger.setthenmakeDir(logdir.value(),"probtrack.log");
	  else
	    logger.makeDir(logdir.value(),"probtrack.log");
	  
	  cout << "Log directory is: " << logger.getDir() << endl;
	  
	  // do again so that options are logged
	  for(int a = 0; a < argc; a++)
	    logger.str() << argv[a] << " ";
	  logger.str() << endl << "---------------------------------------------" << endl << endl;
	}
	
      }
    
      
    }
    catch(X_OptionError& e){
      cerr<<e.what()<<endl;
      cerr<<"try: probtrack --help"<<endl;
      exit(0);
    }
    
    
    
    
  }
  
  
  void probtrackxOptions::modecheck()
  {
    bool check=true;
    string mesg="";
    if(mode.value()=="simple"){
      if(outfile.value()==""){
	mesg+="You must set an output name in simple mode: -o\n";
	check=false;
      }
    }
    /* if(mode.value()=="seedmask"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in seedmask mode: --dir\n";
	check=false;
      }
      if(outfile.value()==""){
	mesg+="You must set an output name in seedmask mode: -o\n";
	check=false;
      }
    }
    if(mode.value()=="twomasks_symm"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in twomasks_symm mode: --dir\n";
	check=false;
      }
      if(mask2.value()==""){
	mesg+="You must set a second mask in twomasks_symm mode: --mask2\n";
	check=false;
      }
      if(outfile.value()==""){
	mesg+="You must set an output name in twomasks_symm mode: -o\n";
	check=false;
      }
    }
    if(mode.value()=="waypoints"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in waypoints mode: --dir\n";
	check=false;
      }
      if(mask2.value()==""){
	mesg+="You must set a waypoint mask or list of waypoint masks in waypoints mode: --mask2\n";
	  check=false;
      }
      if(outfile.value()==""){
	mesg+="You must set output name in waypoints mode: -o\n";
	check=false;
      }
    }
    if(mode.value()=="seeds_to_targets"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in seeds_to_targets mode: --dir\n";
	check=false;
      }
      if(targetfile.value()==""){
	mesg+="You must set a targetmasks file in seeds_to_targets mode: --targetmasks\n";
	check=false;
      }
    }
    if(mode.value()=="matrix1"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in matrix1 mode: --dir\n";
	check=false;
      }
      if(outfile.value()==""){
	mesg+="You must set an output name in matrix1 mode: -o\n";
	check=false;
      }
    }
    
    if(mode.value()=="matrix2"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in matrix2 mode: --dir\n";
	check=false;
      }
      if(outfile.value()==""){
	mesg+="You must set an output name in matrix2 mode: -o\n";
	check=false;
      }
      if(lrmask.value()==""){
	mesg+="You must set a low resolution mask file in matrix2 mode: --lrmask\n";
	check=false;
      }
    }
    
    if(mode.value()=="maskmatrix"){
      if(logdir.value()==""){
	mesg+="You must set an output directory name in maskmatrix mode: --dir\n";
	check=false;
      }
      if(outfile.value()==""){
	mesg+="You must set an output name in maskmatrix mode: -o\n";
	check=false;
      }
      }  */
    if(!check){
      cerr<<mesg;
      exit(2);
    }
    
  }


  
  void probtrackxOptions::modehelp()
  {
    /* cout<<"tracking mode -  Options are:\n\n simple (default)\n    Input is text file defining start point.\n    Output is dti space volume with connectivity values at each voxel.\n   If your seed voxel is not in diffusion space\n    you need to specify a volume in yuour input\n    space (--seedref)\n   and a matrix taking this space to diffusion space(--xfm).\n\n seeds_to_targets\n    Inputs are a volume with ones at all seed points\n    (requires a matrix taking it to DTI space (--xfm))\n    and a text file with the name of a single target mask\n    on each new line (--targetmasks).\n    Output is a single volume for each target mask where the value of each\n    volume within the seed mask corresponds to the number of particles \n    seeded from this voxel reaching this target mask.\n\n seedmask\n    Input is a volume with ones at all seed points\n    (requires a matrix taking it to DTI space (--xfm))\n    Output is seed space volume with connectivity values at each voxel\n    summed from every seed.\n\n twomasks_symm\n    Input is a binary volume for first mask (-x)\n    also requires a binary volume for second mask (--mask2)\n    (requires a matrix taking both seeds to DTI space (--xfm))\n    Output is the sum of all paths the from first mask which pass\n    through the second and vice-versa  i.e. only paths\n    which pass through both masks are retained.\n\n waypoints\n    Input is a binary volume for seed mask (-x)\n    also requires a binary waypoint mask or ascii list\n    of waypoint masks (--mask2)\n    (requires a matrix taking both seeds to DTI space (--xfm))\n    Output is a volume containing all the paths from the\n    seedmask which pass through ALL of the waypoint masks.\n"<<endl;
    if ( getenv("FSLINFMRIB"))
      matrixmodehelp(); 
    */
 }
  
  void probtrackxOptions::matrixmodehelp(){
    cout<<" matrix1\n    Input is a volume with ones at all seed points\n    (requires a matrix taking it to DTI space (--xfm))\n    Output is an avw format nseeds x nseeds matrix \n    where element ij contains the number of particles leaving seed \n    voxel i and passing through seed voxel j. \n    Second output is an nsees x 3 matrix containing the \n    anatomical locations in Seed space of each matrix node.\n\n matrix2\n    Inputs are a volume with ones at all seed points\n    (requires a matrix taking it to DTI space (--xfm))\n    and a low resolution binary mask in register with the\n    seed volume (--lrmask)\n    Output is a matrix of size nseeds x nlowres \n    which contains the connectivity distributions from every seed voxel.\n    Auxiliary outputs are index matrices for each axis \n    of this matrix containing the anatomical location of each\n    node in seed space (x-axis) and low res space (y-axis)\n\n maskmatrix\n    Input is volume with integers in each seed location\n    Integer values distinguish different clusters\n    (e.g. output of Feat clustering)\n    (requires a matrix taking it to DTI space (--xfm))\n    Output is two matrices whose ij^th element is the\n     mean and max number of particles seeded from cluster i which pass \n    through cluster j\n"<<endl;
  }
  
  
  void probtrackxOptions::status()
  {
    cerr<<"basename   "<<basename.value()<<endl;
    cerr<<"maskfile   "<<maskfile.value()<<endl;
    cerr<<"seeds      "<<seedfile.value()<<endl;
    cerr<<"output     "<<outfile.value()<<endl;
    cerr<<"verbose    "<<verbose.value()<<endl;
    cerr<<"nparticles "<<nparticles.value()<<endl;
    cerr<<"nsteps     "<<nsteps.value()<<endl;
    cerr<<"usef       "<<usef.value()<<endl;
    cerr<<"rseed      "<<rseed.value()<<endl; 
  }
  
}










