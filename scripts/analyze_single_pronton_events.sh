#!/bin/sh
while getopts "i:" opt; do
  case $opt in
    i) ir=$OPTARG;;
  esac
done
# Add analysis for IBAInteractionRates!
ir_file=ir_list.txt;
if test -f "$ir_file"; then
  rm -rf $ir_file;
fi

for i in $ir
do
  echo "${i}" >> $ir_file;
  cp  macros/IBAclusterStats.C $i;
  cd $i;
  root.exe -b -q IBAclusterStats.C;
  rm IBAclusterStats.C;
  cd ..;
done
