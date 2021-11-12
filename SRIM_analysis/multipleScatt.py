import numpy as np

# speed of light in /s
c = 3e8 # 

def radiation_lenght():
    # To be done

def getMultipleScatt(Ek_initial, loss_fraction,ion_mass,z,x):
    # We will use the mean energy as a first approximation
    Ek_mean = (Ek_initial + Ek_initial*loss_fraction)/2
    E_rest = ion_mass*c**2
    E_mean = Ek_mean + E_rest
    # The momentum using the spectial relativity
    p = (1/c)*np.sqrt(E_mean**2 - E_rest**2)
    # Need to calculate beta using special relativity
    xi = (Ek_mean + E_rest)/E_rest
    beta = np.sqrt((xi**2 -1)/xi**2)

    

