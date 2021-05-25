#!/bin/sh

while getopts "n:i:" opt; do
  case $opt in
    n) events=$OPTARG;;
    i) ir=$OPTARG;;
  esac
done

#shift 3;
#ir=$@;
echo "Number of events is : $events";
echo "Interaction Rates: $ir";

for j in $ir
do
  mkdir $j;
  cd $j;
  o2-sim -m MFT -e TGeant3 -g boxgen -n $events --configKeyValues 'BoxGun.pdg=2212 ; BoxGun.eta[0]=-20 ; BoxGun.eta[1]=-20.0; BoxGun.number=1; Diamond.position[0]=0.25;Diamond.position[1]=4.004;Diamond.position[2]=-45.000;Diamond.width[2]=0; MFTBase.buildHeatExchanger=false; MFTBase.buildFlex=false';
  o2-sim-digitizer-workflow -b --skipDet TPC,ITS,TOF,FT0,EMC,HMP,ZDC,TRD,MCH,MID,FDD,PHS,FV0,CPV --interactionRate $j --configKeyValues "MFTDigitizerParam.noisePerPixel=0";
  o2-mft-reco-workflow -b;
  cd ..;
done
