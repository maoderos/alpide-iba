#!/bin/bash	

while getopts "i:e:n:" opt; do
  case $opt in
    i) ion=$OPTARG;;
    e) energy=$OPTARG;;
    n) nEvents=$OPTARG;;
  esac
done
	
#ions=(H He Li Be B C N O F Ne);
#energies=(4.0 8.0 10.0 15.0 20.0 30.0 40.0 50.0 60.0 70.0 80.0 90.0 100.0);

if [ ! -d "TRIM_outputs" ]; then
    mkdir TRIM_outputs;
fi
cd TRIM_outputs;
mkdir ${ion}_${energy}MeV_n${nEvents};
cd ${ion}_${energy}MeV_n${nEvents};
cp ../../run_trim_docker.py .; 
docker run -v $PWD:/opt/pysrim/ \
           -v $PWD:/tmp/output \
           costrouc/pysrim sh -c "xvfb-run -a python3.6 /opt/pysrim/run_trim_docker.py ${ion} ${energy} ${nEvents}";
rm run_trim_docker.py;
if [ -f "E2RECOIL.txt" ]; then
    echo "Simulation DONE" > done.txt;
fi




#-it costrouc/pysrim sh -c "xvfb-run -a python3.6 /opt/pysrim/run_trim_docker.py ${ion} ${energy} ${nEvents}";
        
