import numpy as np 
import scipy.interpolate as interpolate	

def find_nearest(array, value):
    array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return array[idx]

def getEnergyStraggling(thickness, range_data, long_strag, kinEn):
   ''' This function finds the energy straggling from the longitudinal straggling using
   SRIM tables. Given the tables of range, longitudinal straggling and initial energy, it gets the corresponding
   longitudinal straggling for a given thickness and search for the range of this value and the its kinectic energy. 
   The function returns the energy straggling, the nearest point of range for the long stragg, the position the thickness given in the table
   and if was necessary to use an interpolation.'''
   
   # fist, we need to find the position of range_data thickness
   value_near = find_nearest(range_data, thickness)
   print(value_near)
   pos, = np.where(range_data == value_near)

   # Now we find the nearest point of range_data in tables given the longitudinal
   # straggling 
   print(pos)
   near_point = find_nearest(range_data, long_strag[pos])

   need_interpol = True
   near_p_idx, = np.where(range_data == near_point)
   if near_point > long_strag[pos]:
      range_data_interp = range_data[near_p_idx[0]-1:near_p_idx[0] + 1]
      kinEn_interp = kinEn[near_p_idx[0]-1:near_p_idx[0] + 1]
      range_data_graph = range_data[near_p_idx[0]-3:near_p_idx[0] + 4]
      kinEn_graph = kinEn[near_p_idx[0]-3:near_p_idx[0] + 4]
   elif near_point < long_strag[pos]:
      range_data_interp = range_data[near_p_idx[0]:near_p_idx[0] + 2]
      kinEn_interp = kinEn[near_p_idx[0]:near_p_idx[0] + 2]
      range_data_graph = range_data[near_p_idx[0]-2:near_p_idx[0] + 4]
      kinEn_graph = kinEn[near_p_idx[0]-2:near_p_idx[0] + 4]
   elif near_point == long_strag[pos]:
      need_interpol = False
      energy_straggling_index, = np.where(range_data == long_strag[pos])
      energy_straggling = kinEn[energy_straggling_index[0]]
    
    # if the nearest point doesnt appear in table, intepolate to estimate kinEn

   if (need_interpol):
      interp_function = interpolate.interp1d(range_data_interp, kinEn_interp, kind='linear')
      energy_straggling = interp_function(long_strag[pos])

    #Fraction of energy loss given the initial energy
    
   return energy_straggling, pos, need_interpol, near_point, value_near
