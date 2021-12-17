import numpy as np 
import scipy.interpolate as interpolate	

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


def calculateMomentum(K, E_rest):
    ''' Calculate the momentum using E^2 = (pc)^2 + E_rest^2'''
    E = K + E_rest
    return (np.sqrt( E**2 - E_rest**2))
    
def calculateBeta(K,E_rest):
    '''Calculate the Beta in function of the energy'''

    xi = (K + E_rest)/E_rest
    return np.sqrt( (xi**2 - 1)/(xi**2) )

def calculate_energyLossFraction(data_arr,de_dx,range_data,kinEn,energy,thickness,ion_mass,radiation_length,weigth,density,z=1):
    energy_pos, = np.where(kinEn == energy)
    # print(range_data[energy_pos[0]])
    diff = range_data[energy_pos[0]] - thickness
    if(diff >= range_data[0]):


        #Now interpolate the data of dE/dx vs k

        interp_function = interpolate.interp1d(kinEn[:energy_pos[0] + 1], de_dx[:energy_pos[0] + 1], kind='cubic')
        interp_function_elec = interpolate.interp1d(data_arr[:,0], data_arr[:,1], kind='cubic')
        interp_function_nuc = interpolate.interp1d(data_arr[:,0], data_arr[:,2], kind='cubic')
        
        '''
        plt.title("He")
        plt.plot(kinEn[:energy_pos[0]+1], interp_function(kinEn[:energy_pos[0] + 1]),label='cubic interpol')
        plt.plot(kinEn[:energy_pos[0] + 1], de_dx[:energy_pos[0]+1],'o',label="SRIM-2013")
        plt.xlabel("Kinetic Energy(MeV)")
        plt.ylabel("dE/dx(MeV)")
        plt.yscale('log')
        plt.xscale('log')
        plt.legend()
        plt.show()
        '''
        init=0.0
        step=1.0
        E = energy
        E_final = 0
        E_rest = ion_mass
        E_nuc = 0
        E_elec = 0
        #calculate Xo mix
        Xo_mix = getRadiation_length(radiation_length, weigth, density)
        sum1 = 0
        sum2 = 0
        while(init < thickness):

            beta = calculateBeta(E,E_rest)
            p = calculateMomentum(E,E_rest)
            sum1 += ( (step*1e-4)/(Xo_mix) )*( 1/(p*p*beta*beta) )
            sum2 += ( (step*1e-4)/(Xo_mix) )*( (z**2)/(beta**2) )
            E_loss = interp_function(E)*step*(1e-03) #Kev --> MeV
            E_loss_elec = interp_function_elec(E)*step*(1e-03) #Kev --> MeV
            E_loss_nuc = interp_function_nuc(E)*step*(1e-03) #Kev --> MeV
            E_final += E_loss
            E_elec += E_loss_elec
            E_nuc += E_loss_nuc
            E -= E_loss

            init+=step

        theta = 14.1*z*np.sqrt(sum1)*(1 + (1/9)*np.log10(sum2))*57.2957 # to change to degrees
        #frac_finalEn = (E_final/energy)
        return E_final/energy, E_elec, E_nuc,theta 
    else:
        return -1,-10,-10,1000
