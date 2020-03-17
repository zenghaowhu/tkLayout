#!/bin/bash

export configurationdir=/cefs/higgs/zengh/tkLayout/cepclayout/CEPC_V4
VTXname="V4_1_0_0_r53"
bpixname="4_1_0_r53"
templatecfg="OT_V4_100_IT_V4_1_0_0.cfg"
#pixelcfgtmp=${templatecfg%.cfg}
templatepixel=Pixel_${templatecfg#*IT_}
templatebpix=BPIX_${templatecfg:14:4}${templatecfg:20}
cp -rf ${configurationdir}/$templatecfg ${configurationdir}/${templatecfg%IT*}IT_${VTXname}.cfg
sed -i "s#${templatepixel}#Pixel_${VTXname}.cfg#g" ${configurationdir}/${templatecfg%IT*}IT_${VTXname}.cfg

cp -rf ${configurationdir}/Pixel/${templatepixel} ${configurationdir}/Pixel/Pixel_${VTXname}.cfg
sed -i "s#${templatebpix}#BPIX_${bpixname}.cfg#g" ${configurationdir}/Pixel/Pixel_${VTXname}.cfg

cp -rf ${configurationdir}/Pixel/${templatebpix} ${configurationdir}/Pixel/BPIX_${bpixname}.cfg

