#include "newimage/newimageall.h"
#include <host_vector.h>
#include <device_vector.h> 

#include "fibre_gpu.h"

//implemented and used in xfibres_gpu.cu
void fit(	//INPUT
		const vector<ColumnVector> 	datam_vec, 
		const vector<Matrix> 		bvecs_vec,
		const vector<Matrix> 		bvals_vec,
		thrust::host_vector<double> 	datam_host,	
		thrust::host_vector<double>	bvecs_host, 
		thrust::host_vector<double>	bvals_host,
		thrust::device_vector<double> 	datam_gpu, 
		thrust::device_vector<double>	bvecs_gpu, 
		thrust::device_vector<double>	bvals_gpu,
		string 				output_file, 
		//OUTPUT
		thrust::device_vector<double>&	params_gpu,
		thrust::host_vector<int>&	vox_repeat,
		int&				nrepeat);

//implemented and used in xfibres_gpu.cu
void prepare_data_gpu_FIT(	//INPUT
				const Matrix				datam,
				const Matrix				bvecs,
				const Matrix				bvals,
				const Matrix	 			gradm, 
				const Matrix 				Qform, 
				const Matrix 				Qform_inv,
				//OUTPUT
				vector<ColumnVector>&			datam_vec,
				vector<Matrix>&				bvecs_vec,
				vector<Matrix>&				bvals_vec,
				thrust::host_vector<double>&   		datam_host,	
				thrust::host_vector<double>&		bvecs_host,				
				thrust::host_vector<double>&		bvals_host,
				thrust::host_vector<double>&		alpha_host,
				thrust::host_vector<double>&		beta_host,
				thrust::host_vector<double>&		params_host,
				thrust::host_vector<float>&		tau_host);


//implemented and used in xfibres_gpu.cu
void prepare_data_gpu_FIT_repeat(	//INPUT
					thrust::host_vector<double>   		datam_host,	
					thrust::host_vector<double>		bvecs_host,				
					thrust::host_vector<double>		bvals_host,
					thrust::host_vector<int>		vox_repeat,
					int					nrepeat,
					//OUTPUT
					vector<ColumnVector>&			datam_repeat_vec,
					vector<Matrix>&				bvecs_repeat_vec,
					vector<Matrix>&				bvals_repeat_vec,
					thrust::host_vector<double>&   		datam_repeat_host,
					thrust::host_vector<double>&		bvecs_repeat_host,				
					thrust::host_vector<double>&		bvals_repeat_host,
					thrust::host_vector<double>&		params_repeat_host);

//implemented and used in xfibres_gpu.cu
void mix_params(	//INPUT
			thrust::device_vector<double>   		params_repeat_gpu,
			thrust::host_vector<int>			vox_repeat,
			int						nrepeat,
			//INPUT-OUTPUT
			thrust::device_vector<double>&   		params_gpu);


//implemented and used in xfibres_gpu.cu
void prepare_data_gpu_MCMC(	//INPUT
				int 					nvox,
				int 					nfib,
				//OUTPUT
				thrust::host_vector<double>&		signals_host,
				thrust::host_vector<double>&		isosignals_host,
				thrust::host_vector<FibreGPU>& 		fibres_host,
				thrust::host_vector<MultifibreGPU>& 	multifibres_host);

//implemented and used in xfibres_gpu.cu
void prepare_data_gpu_MCMC_record(	//INPUT
					int 						nvox,
					//OUTPUT
					thrust::device_vector<int>&			multirecords_gpu,
					thrust::device_vector<float>&			rf0_gpu,
					thrust::device_vector<float>&			rtau_gpu,
					thrust::device_vector<float>&			rs0_gpu,
					thrust::device_vector<float>&			rd_gpu,
					thrust::device_vector<float>&			rdstd_gpu,
					thrust::device_vector<float>&			rth_gpu,
					thrust::device_vector<float>&			rph_gpu,
					thrust::device_vector<float>&			rf_gpu,
					thrust::device_vector<float>&			rlikelihood_energy_gpu);

//implemented and used in xfibres_gpu.cu
void record_finish_voxels(	//INPUT
				const NEWIMAGE::volume<int> 			vol2matrixkey,
				const NEWMAT::Matrix				matrix2volkey,
				const NEWIMAGE::volume<float>			mask,
				thrust::device_vector<int>&			multirecords_gpu,
				thrust::device_vector<float>&			rf0_gpu,
				thrust::device_vector<float>&			rtau_gpu,
				thrust::device_vector<float>&			rs0_gpu,
				thrust::device_vector<float>&			rd_gpu,
				thrust::device_vector<float>&			rdstd_gpu,
				thrust::device_vector<float>&			rth_gpu,
				thrust::device_vector<float>&			rph_gpu,
				thrust::device_vector<float>&			rf_gpu,
				thrust::device_vector<float>&			rlikelihood_energy_gpu,
				int 						nvox);
