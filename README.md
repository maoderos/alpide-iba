# ALPIDE sensor for Low Energy Ions 

Studies on ALPIDE sensor performance at low energies regime and its potecial for Ion Beam Analysis.

* `macros/IBAclusterStats.C`: Some statistics on clusters production, hits and ROFs
  * `$ root.exe -q -l IBAclusterStats.C`
* `macros/IBAInteractionRate.C`: Analysis of valid clusters as function of the interaction Rate.
  * `$ root.exe -q -l IBAInteractionRate.C`
* `macros/IBAInteractionFluence.C`:  Efficiency analysis in terms of fluence.
  * `$ root.exe -q -l IBAInteractionFluence.C`
* `macros/analysisEff.C`: Fit efficiency in energy threshold region with a gaussian CDF.
* `scripts/Generate_single_proton_events.sh`: Generate proton events for each interaction rate provided.
* `scripts/plot_eff.sh`: Plot efficiency as function of interaction rate using the macro "IBAInteractionRate.C".
* `o2-files/`: Files from o2 simulation that were modified. 
* `SRIM_analys/`: SRIM simulation data and codes to analyze its data. In `SRIM_analysis.py` we have the estimation of energy straggling from a longitudinal straggling and the fraction of energy loss crossing the ALPIDE.
* `TRIM_analysys`: Python codes for TRIM simulation and data analysis of target damage of the ALPIDE model for different ions and energies.

## O2 Simulation Instructions

OBS: The folder "alpide-iba" must be in the directory you want to make the simulation.

To simulate and analyze its data with "IBAclusterStats.C":
* `./alpide-iba/scripts/Generate_single_proton_events.sh -n 10 -i "10 20 30"`

Where `-n` is the number of events and `-i` are the interaction rates that you want to make the simulation. For each interaction rate, one folder with the corresponding number will be created. 

OBS:In order to decrease the time of simulation, the command above has the following strategy: If the number of events is less than 2000 or odd, the script will make just 1 simulation with the number provided. Otherwise, if the number of events is bigger than 2000 and even, (n/2000) simulations will be made. 

For generating the plot of efficiency (nClusterFromPrimaries/nPrimaries), you need to specify which interaction rates do you want to use to make the plot:

* `./scripts/plot_eff.sh -i "10 20 30"`

The command above will generate the efficiency by interaction rate plot.

## TRIM Analysis

For using the python codes generated for analysing the damage, it is necessary to have SRIM installed and also the pysrim library.
* pysrim: `https://pypi.org/project/pysrim/`
* SRIM: `http://srim.org/`
* Tutorial for installing SRIM in linux platforms with wine: `https://www.researchgate.net/publication/324329665_Installing_SRIM_2013_on_linuxmint_mate` 
