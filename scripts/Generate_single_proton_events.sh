#!/bin/bash

workers=4;
efficiency=false;
energy_plot=false;

while getopts "n:i:efj:g:" opt; do
  case $opt in
    n) events=$OPTARG;;
    i) ir=$OPTARG;;
    j) workers=$OPTARG;;
    e) energy_plot=true;;
    f) efficiency=true;;
    g) geant=$OPTARG;
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
  cp alpide-iba/energyLossFormat.txt $i;
  cd $i;
  echo "${nDivisions}" > nSim.txt;
  for (( nEv=1; nEv<=$nDivisions; nEv++ ))
  do
    mkdir $nEv;
    cd $nEv;
    cp ~/alice/MFTdictionary.bin .;
    o2-sim -m MFT -e $geant -g boxgen -j $workers -n $dEvents --configKeyValues 'BoxGun.pdg=2212 ; BoxGun.eta[0]=179.70 ; BoxGun.eta[1]=180.00; BoxGun.prange[0]=1.000; BoxGun.prange[1]=6.00; BoxGun.number=100; Diamond.position[0]=-2.00;Diamond.position[1]=-3.546;Diamond.position[2]=80.000;Diamond.width[2]=0; MFTBase.buildHeatExchanger=false; MFTBase.buildFlex=false; MFTBase.minimal=true;';
    o2-sim-digitizer-workflow -b --skipDet TPC,ITS,TOF,FT0,EMC,HMP,ZDC,TRD,MCH,MID,FDD,PHS,FV0,CPV --interactionRate $i --configKeyValues "MFTDigitizerParam.noisePerPixel=0";
    o2-mft-reco-workflow -b;
    cd ..;
  done
  if $energy_plot; then
     root.exe -b -q ../alpide-iba/macros/IBAenergyAnalysis.C;
  fi
  if $efficiency; then
     root.exe -b -q ../alpide-iba/macros/IBAclusterStats.C;
  fi
  cd ..;
done
duration=$(( SECONDS - start ))
echo "Time of SIM: ${duration}s";
