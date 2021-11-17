import numpy as np 
import scipy.interpolate as interpolate	

def calculate_energyLossFraction(de_dx,range_data,kinEn,energy,thickness):
    energy_pos, = np.where(kinEn == energy)
   # print(range_data[energy_pos[0]])
    diff = range_data[energy_pos[0]] - thickness
    if(diff >= range_data[0]):
        '''

        interp_function = interpolate.interp1d(range_data[:energy_pos[0] + 1], de_dx[:energy_pos[0] + 1], kind='cubic')

        
        plt.plot(range_data[:energy_pos[0]+1], interp_function(range_data[:energy_pos[0] + 1]),label='cubic interpol')
        plt.plot(range_data[:energy_pos[0] + 1], de_dx[:energy_pos[0]+1],'o',label="SRIM-2013")
        plt.xlabel("range($\mu$m)")
        plt.ylabel("dE/dx(MeV)")
        plt.legend()
        plt.show()
        '''

        #Now interpolate the data of dE/dx vs k

        interp_function = interpolate.interp1d(kinEn[:energy_pos[0] + 1], de_dx[:energy_pos[0] + 1], kind='cubic')
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
        while(init < thickness):
           
            E_loss = interp_function(E)*step*(1e-03)
            E_final += E_loss
            E -= E_loss

            init+=step

        
        return((E_final)/energy)
    else:
        return 0
