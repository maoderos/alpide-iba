# ALPIDE sensor for IBA

* `macro/IBAclusterStats.C`: Some statistics on clusters production
  * `$ root.exe -q IBAclusterStats.C`
* `macro/IBAInteractionRate.C`: Analysis of valid clusters as function of the interaction Rate.
  * `$ root.exe -q IBAInteractionRate.C`
* `scripts/Generate_single_proton_events.sh`: Generate proton events for each interaction rate provided.
* `scripts/plot_eff.sh`: Plot efficiency as function of interaction rate using the macro "IBAInteractionRate.C".
* `o2-files/`: Files from o2 simulation that were modified. 

## Simulation Instructions

OBS: The folder "alpide-iba" must be in the directory you want to make the simulation.

To simulate and analyze its data with "IBAclusterStats.C":
* `./alpide-iba/scripts/Generate_single_proton_events.sh -n 10 -i "10 20 30"`

Where `-n` is the number of events and `-i` are the interaction rates that you want to make the simulation. For each interaction rate, one folder with the corresponding number will be created. 

OBS:In order to decrease the time of simulation, the command above has the following strategy: If the number of events is less than 2000 or odd, the script will make just 1 simulation with the number provided. Otherwise, if the number of events is bigger than 2000 and even, (n/2000) simulations will be made. That's because we observed that as the number of events increase, slower is the simulation. 

For generating the plot of efficiency (nClusterFromPrimaries/nPrimaries), you need to specify which interaction rates do you want to use to make the plot:

* `./scripts/plot_eff.sh -i "10 20 30"`

The command above will generate the efficiency by interaction rate plot.
