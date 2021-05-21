# ALPIDE sensor for IBA

* `macro/IBAclusterStats.C`: Some statistics on clusters production
  * `$ root.exe -q IBAclusterStats.C+`
* `macro/IBAInteractionRate.C`: Analysis of valid clusters as function of the interaction Rate.
  * `$ root.exe -q IBAInteractionRate.C+`
* `scripts/Generate_single_proton_events.sh`: Generate proton events for each interaction rate provided.
* `scripts/analyze_single_pronton_events.sh`: Apply root macro for simulations.

## Simulation Instructions

Obs: These instruction will work only inside O2 environment and the folders "macros" and "scripts" must be in the same directory.

For generating events using the `Generate_single_proton_events.sh`:
* `source scripts/Generate_single_proton_events.sh -n 10 number_events -i "10 20 30"`

The command above generate 3 simulations with interaction rates of 10hz,20hz and 30hz with 10 events. 3 folders will be created, each of them with the name of the interaction rate.

For analysing the data using `analyze_single_pronton_events.sh`:
* `scripts/analyze_single_pronton_events.sh -i "10 20 30"`

The command above execute the `IBAclusterStats.C` inside folder 10,20 and 30, generating hitograms and a root file called "ALPIDESinglePixel.root".
