#!/bin/bash

geotracks=1000
materialtracks=1000

export configurationdir=/cefs/higgs/zengh/tkLayout/cepclayout/CEPC_full_silicon
export jobdir=/cefs/higgs/zengh/tkLayout/cepclayout/CEPC_full_silicon/jobfile
echo "configuration directory is:" 
echo $configurationdir
echo "job directory is:"
echo $jobdir
cd $configurationdir
echo "Now we are in:"
pwd
for file in *.cfg
do

	    InputFile=${file}
	    echo "input configuration file:"
	    echo $InputFile
	    if [ -f "$InputFile" ]; then
            tklayoutInputFile=$configurationdir/$InputFile
            jobfile=job_${InputFile%.cfg}

	        rm -f $jobdir/${jobfile}.sh
	        cp -fr ${jobdir}/jobtemplate.sh ${jobdir}/${jobfile}.sh

	        sed -i "s#CONFIGFILE#$tklayoutInputFile#g" ${jobdir}/${jobfile}.sh
	        sed -i "s#GeoTrack#$geotracks#g" ${jobdir}/${jobfile}.sh
	        sed -i "s#MatTrack#$materialtracks#g" ${jobdir}/${jobfile}.sh
	        echo "${jobdir}/${jobfile}.sh"
	        chmod +x ${jobdir}/${jobfile}.sh
	        hep_sub ${jobdir}/${jobfile}.sh
	        #sh $RecoWorkDir/$OUTPUTDATA/reco_${par}_${sjob}.sh
        fi

done



