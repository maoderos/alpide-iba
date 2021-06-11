#!/bin/sh
while getopts "i:" opt; do
  case $opt in
    i) ir=$OPTARG;;
  esac
done
# Add analysis for IBAInteractionRates!

if [ $((events%2)) -eq 0 ]
then
  echo "even number. OK";
else
  echo "must be an even number of events"
  exit;
fi


nDivisions=$events/2000;

for i in $ir
do
  cp  macros/IBAclusterStats.C $i;
  cd $i;
  root.exe -b -q IBAclusterStats.C;
  rm IBAclusterStats.C;
  cd ..;
done
