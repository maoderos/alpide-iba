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
  nDivisions=$((events/dEvents));
else
  echo "Number of events is less than 2000 or is a odd number. then we will run the full simulation";
  nDivisions=1;
  dEvents=$events;
fi

start=$SECONDS;

for i in $ir
do
  mkdir $i;
  cp ~/alpide-iba/macros/IBAclusterStats.C $i;
  cd $i;
  echo "${nDivisions}" > nSim.txt;
  for (( nEv=1; nEv<=$nDivisions; nEv++ ))
  do
    mkdir $nEv;
    cd $nEv;
    cp ~/alice/MFTdictionary.bin .;
    o2-sim -m MFT -e TGeant3 -g boxgen -j $workers -n $dEvents --configKeyValues 'BoxGun.pdg=2212 ; BoxGun.eta[0]=180.02 ; BoxGun.eta[1]=180.30; BoxGun.prange[0]=1; BoxGun.prange[1]=6; BoxGun.number=100; Diamond.position[0]=-2.00;Diamond.position[1]=-3.546;Diamond.position[2]=80.000;Diamond.width[2]=0; MFTBase.buildHeatExchanger=false; MFTBase.buildFlex=false; MFTBase.minimal=true;';
    o2-sim-digitizer-workflow -b --skipDet TPC,ITS,TOF,FT0,EMC,HMP,ZDC,TRD,MCH,MID,FDD,PHS,FV0,CPV --interactionRate $i --configKeyValues "MFTDigitizerParam.noisePerPixel=0";
    o2-mft-reco-workflow -b;
    cd ..;
  done
  root.exe -b -q IBAclusterStats.C;
  cd ..;
done
duration=$(( SECONDS - start ))
echo "Time of SIM: ${duration}s";
