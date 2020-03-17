#!/bin/bash
cd /cefs/higgs/zengh/tkLayout/
source setup_slc6.sh
#tklayout /cefs/higgs/zengh/tkLayout/cepclayout/CEPC_V4/OT_V4_100_IT_V4_1_0_04_nogap.cfg -n 100000 -N 5000 -a
tklayout /cefs/higgs/zengh/tkLayout/geometries/CMS_Phase2/FlatTracker4026.cfg -n 5000 -N 3000 -a
