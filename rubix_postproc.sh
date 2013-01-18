#!/bin/sh

out_dir=$1
dataLR=$2
maskLR=$3
dataHR=$4


numfib=`${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_0000/f*samples* | wc -w | awk '{print $1}'`

fib=1
while [ $fib -le $numfib ]
do
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/merged_th${fib}samples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/th${fib}samples*`
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/merged_ph${fib}samples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/ph${fib}samples*`
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/merged_f${fib}samples  `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/f${fib}samples*`
    ${FSLDIR}/bin/fslmaths ${out_dir}/merged_f${fib}samples -Tmean ${out_dir}/mean_f${fib}samples
    ${FSLDIR}/bin/make_dyadic_vectors ${out_dir}/merged_th${fib}samples ${out_dir}/merged_ph${fib}samples ${out_dir}/mean_f${fib}samples ${out_dir}/dyads${fib} 95
    fib=$(($fib + 1))
done


if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_dsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_dsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_dsamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_S0samples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_S0samples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_S0samples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_tausamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_tausamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_tausamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_tau_LRsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_tau_LRsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_tau_LRsamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_S0_LRsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_S0_LRsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_S0_LRsamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/En_Lik_LRsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/En_Lik_LRsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/En_Lik_LRsamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/En_Prior_LRsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/En_Prior_LRsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/En_Prior_LRsamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_dstd_samples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_dstd_samples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_dstd_samples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/HRbrain_mask` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/HRbrain_mask `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/HRbrain_mask*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_fsumPriorMode_LRsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_fsumPriorMode_LRsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_fsumPriorMode_LRsamples*`
fi

if [ `${FSLDIR}/bin/imtest ${out_dir}/diff_slices/data_slice_0000/mean_dPriorMode_LRsamples` -eq 1 ];then
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/mean_dPriorMode_LRsamples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/mean_dPriorMode_LRsamples*`
fi

numModes=`${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_0000/Pk*samples* | wc -w | awk '{print $1}'`

mode=1
while [ $mode -le $numModes ]
do
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/merged_Pth${mode}samples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/Pth${mode}samples*`
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/merged_Pph${mode}samples `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/Pph${mode}samples*`
    ${FSLDIR}/bin/fslmerge -z ${out_dir}/merged_Pk${mode}samples  `${FSLDIR}/bin/imglob ${out_dir}/diff_slices/data_slice_*/Pk${mode}samples*`
    ${FSLDIR}/bin/fslmaths ${out_dir}/merged_Pk${mode}samples -Tmean ${out_dir}/mean_Pk${mode}samples
    ${FSLDIR}/bin/make_dyadic_vectors ${out_dir}/merged_Pth${mode}samples ${out_dir}/merged_Pph${mode}samples ${out_dir}/mean_Pk${mode}samples ${out_dir}/Pdyads${mode}
    mode=$(($mode + 1))
done


echo Removing intermediate files

if [ `imtest ${out_dir}/merged_th1samples` -eq 1 ];then
  if [ `imtest ${out_dir}/merged_ph1samples` -eq 1 ];then
    if [ `imtest ${out_dir}/merged_f1samples` -eq 1 ];then
      rm -rf ${out_dir}/diff_slices
      rm -f ${out_dir}/${dataLR}_slice_*
      rm -f ${out_dir}/${dataHR}_newslice_* 
      rm -f ${out_dir}/${maskLR}_slice_*
    fi
  fi
fi

echo Done