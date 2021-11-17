import numpy as np

# speed of light in /s
c = 3e8 # 

def getRadiation_length(Xo, weigth, density):
    ''' Function that calculates the radiation length of a mixure.
    Parameters: Xo - Radiation length
                weigth - fraction of mass of mixture
                density - density of the material'''
    n_list = len(Xo)
    reciprocal_Xo = 0
    for i in range(0,n_list):
        reciprocal_Xo += weigth[i]/Xo[i]
    
    Xo_mix = (1/reciprocal_Xo)

    #return in cm, divide by the density 

    return Xo_mix/density


def getMultipleScatt(Ek_initial, loss_fraction,ion_mass,z,x, radiation_length, weigth, density):
    # We will use the mean energy as a first approximation
    Ek_mean = (Ek_initial + Ek_initial*loss_fraction)/2
    E_rest = ion_mass # cause mass is in MeV/c**2
    E_mean = Ek_mean + E_rest
    # The momentum using special relativity
    p = np.sqrt(E_mean**2 - E_rest**2) #MeV/c
    # Need to calculate beta using special relativity
    xi = (Ek_mean + E_rest)/E_rest
    beta = np.sqrt((xi**2 - 1)/(xi**2))

    Xo_mix = getRadiation_length(radiation_length, weigth, density)
    #print("Radiation length = ", Xo_mix, " cm")

    #print("x/Xo = ", (x*1e-4)/(Xo_mix))

    return (13.6/(beta*p))*z*np.sqrt((x*1e-4)/Xo_mix)*(1 + 0.038*np.log((x*z**2)/(Xo_mix*beta**2) ) )*57.2957 # to change to degrees
    







    

