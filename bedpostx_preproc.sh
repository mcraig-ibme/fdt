#!/bin/sh

subjdir=$1

echo Copying files to bedpost directory - ya cock
cp ${subjdir}/bvecs ${subjdir}/bvals ${subjdir}.bedpostX
${FSLDIR}/bin/imcp ${subjdir}/nodif_brain_mask ${subjdir}.bedpostX
${FSLDIR}/bin/fslmaths\
 ${subjdir}/nodif\
 -mas ${subjdir}/nodif_brain_mask\
 ${subjdir}.bedpostX/nodif_brain

${FSLDIR}/bin/fslslice ${subjdir}/data
${FSLDIR}/bin/fslslice ${subjdir}/nodif_brain_mask
echo Done
