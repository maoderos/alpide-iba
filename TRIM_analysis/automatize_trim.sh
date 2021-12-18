#!/bin/sh

ions=(H He Li Be B C N O F Ne);
energies=(4.0 8.0 10.0 15.0 20.0 30.0 40.0 50.0 60.0 70.0 80.0 90.0 100.0);

mkdir TRIM_outputs;
cd TRIM_outputs;

for ion in ${ions[@]}; do
   mkdir $ion;
   cd $ion;
   for energy in ${energies[@]}; do 
      mkdir $energy;
      cd $energy;
      python3 ../../../run_trim.py $ion $energy;
      cd ..;  
   done
   cd ..;
done



