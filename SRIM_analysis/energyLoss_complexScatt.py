import numpy as np 
import scipy.interpolate as interpolate	

def getRadiation_length(Xo, weigth, density):
    ''' Function that calculates the radiation length of a mixure.
        Parameters: Xo - Radiation length
        weigth - fraction of mass of mixture
        density - density of the material'''
    n_list = len(weigth)
    reciprocal_Xo = 0
    for i in range(0,n_list):
        reciprocal_Xo += weigth[i]/Xo[i]
        Xo_mix = (1/reciprocal_Xo)
    #return in cm, divide by the density 
    return Xo_mix/density


def calculateMomentum(K, E_rest):
    ''' Calculate the momentum using E^2 = (pc)^2 + E_rest^2'''
    E = K + E_rest
    return (np.sqrt( E**2 - E_rest**2))
    
def calculateBeta(K,E_rest):
    '''Calculate the Beta in function of the energy'''

    xi = (K + E_rest)/E_rest
    return np.sqrt( (xi**2 - 1)/(xi**2) )
    

def calculate_energyLossFraction(data_metal,data_si,energy,thickness,ion_mass,radiation_length,weigth_metal, weigth_si,density_metal,density_si,z=1):
    #energy_pos, = np.where(kinEn == energy)
    # print(range_data[energy_pos[0]])
    #diff = range_data[energy_pos[0]] - thickness
    init=0.0
    step=1.0
    E = energy
    E_final = 0
    E_nuc = 0 
    E_elec = 0
    E_rest = ion_mass
    sum1 = 0
    sum2 = 0
    while(init < thickness):
        if(E > 0):
            if(init>=10.0): # until 10 um ih the metal stack.
                kinEn = data_si[:,0]
                dE_dx = np.add(data_si[:,1],data_si[:,2])
                dE_dx_elec = data_si[:,1]
                dE_dx_nuc = data_si[:,2]
                Xo = getRadiation_length([radiation_length[0]], weigth_si, density_si)
            else:
                kinEn = data_metal[:,0]
                dE_dx = np.add(data_metal[:,1],data_metal[:,2])
                dE_dx_elec = data_metal[:,1]
                dE_dx_nuc = data_metal[:,2]
                Xo = getRadiation_length(radiation_length, weigth_metal, density_metal)
            
            interp_function = interpolate.interp1d(kinEn, dE_dx, kind='cubic')
            interp_function_elec = interpolate.interp1d(kinEn, dE_dx_elec, kind='cubic')
            interp_function_nuc = interpolate.interp1d(kinEn, dE_dx_nuc, kind='cubic')
            beta = calculateBeta(E,E_rest)
            p = calculateMomentum(E,E_rest)
            sum1 += ( (step*1e-4)/(Xo) )*( 1/(p*p*beta*beta) )
            sum2 += ( (step*1e-4)/(Xo) )*( (z**2)/(beta**2) )
            try:
                interp_function(E)
            except:
                return -energy, -10, -10, 1000
            E_loss = interp_function(E)*step*(1e-03) #Kev --> MeV
            E_loss_elec = interp_function_elec(E)*step*(1e-03)
            E_loss_nuc = interp_function_nuc(E)*step*(1e-03)
            E_final += E_loss
            E_elec += E_loss_elec
            E_nuc +=  E_loss_nuc 
            E -= E_loss
            init+=step
            if(E<0):
                return -energy, -10, -10, 1000
            
        #frac_finalEn = (E_final/energy)
        else:
            return -energy, -10, -10, 1000
    theta = 14.1*z*np.sqrt(sum1)*(1 + (1/9)*np.log10(sum2))*57.2957 # to change to degrees
    return E_final, E_elec ,E_nuc, theta 
