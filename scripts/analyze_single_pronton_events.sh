#!/bin/sh
while getopts "i:" opt; do
  case $opt in
    i) ir=$OPTARG;;
  esac
done
# Add analysis for IBAInteractionRates!
for i in $ir
do
  cp  macros/IBAclusterStats.C $i;
  cd $i;
  root.exe -b IBAclusterStats.C;
  rm IBAclusterStats.C;
  cd ..;
done
