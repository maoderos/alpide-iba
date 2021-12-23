#!/bin/sh

while getopts "i:e:n:" opt; do
  case $opt in
    i) ions=$OPTARG;;
    e) energies=$OPTARG;;
    n) nEvents=$OPTARG;;
  esac
done

#ions=(H He Li Be B C N O F Ne);
#energies=(4.0 8.0 10.0 15.0 20.0 30.0 40.0 50.0 60.0 70.0 80.0 90.0 100.0);

mkdir TRIM_outputs;
cd TRIM_outputs;

for ion in $ions; do
   mkdir $ion;
   cd $ion;
   for energy in $energies; do 
      mkdir $energy;
      cd $energy;
      cp ../../../run_trim_docker.py .; 
      docker run -v $PWD:/opt/pysrim/ \
             -v $PWD:/tmp/output \
             -it costrouc/pysrim sh -c "xvfb-run -a python3.6 /opt/pysrim/run_trim_docker.py $ion $energy $nEvents";
      rm run_trim_docker.py;
      cd ..;  
   done
   cd ..;
done



        
