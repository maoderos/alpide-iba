#!/bin/bash

workers=4;

while getopts "n:i:j:" opt; do
  case $opt in
    n) events=$OPTARG;;
    i) ir=$OPTARG;;
    j) workers=$OPTARG;;
  esac
done

#shift 3;
#ir=$@;
echo "Number of events is : $events";
echo "Interaction Rates: $ir";


#events for each folder
dEvents=2000;


if [ $((events%dEvents)) -eq 0 ]
then
  echo "OK";
else
  echo "Tota events must be multiple of single events"
  exit;
fi

nDivisions=$((events/dEvents));
start=$SECONDS;

for i in $ir
do
  mkdir $i;
  cd $i;
  echo "${nDivisions}" > nSim.txt;
  for (( nEv=1; nEv<=$nDivisions; nEv++ ))
  do
    mkdir $nEv;
    cd $nEv;
    o2-sim -m MFT -e TGeant3 -g boxgen -j $workers -n $dEvents --configKeyValues 'BoxGun.pdg=2212 ; BoxGun.eta[0]=-20 ; BoxGun.eta[1]=-20.0; BoxGun.number=1; Diamond.position[0]=-0.30;Diamond.position[1]=-3.850;Diamond.position[2]=-45.000;Diamond.width[2]=0; MFTBase.buildHeatExchanger=false; MFTBase.buildFlex=false; MFTBase.minimal=true;';
    o2-sim-digitizer-workflow -b --skipDet TPC,ITS,TOF,FT0,EMC,HMP,ZDC,TRD,MCH,MID,FDD,PHS,FV0,CPV --interactionRate $i --configKeyValues "MFTDigitizerParam.noisePerPixel=0";
    o2-mft-reco-workflow -b;
    cd ..;
  done
  cd ..;
done
duration=$(( SECONDS - start ))
echo "Time of SIM: ${duration}s";
