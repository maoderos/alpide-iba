
import os
import sys
import re
import numpy as np
from io import StringIO 
import matplotlib.pyplot as plt
import scipy.interpolate as interpolate
import scipy.integrate as integrate




#--------Initial values----------

elements = ["H", "He", "Li" ,"Be", "B", "C", "N", "O", "F", "Ne"]
initial_energy = [4.0,8.0,10.0,15.0,20.0,30.0,40.0,50.0,60.0,70.0,80.0,90.0,100.0] # MeV
radical = "alpide.txt"
thickness = 50.0 #in um
EnergyLossTable = True
#elements=["He"]
#initial_energy=[10.0]


def calculate_energyLossFraction(de_dx,range_data,kinEn,energy):
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

        #Now we use the interpolation function to integrate between the points

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
        step=0.5
        E = energy
        E_final = 0
        while(init < thickness):
           
            E_loss = interp_function(E)*step*(1e-03)
            E_final += E_loss
            E -= E_loss

            init+=step

        
        #energyLoss = integrate.quad(interp_function,diff,range_data[energy_pos[0]])
        #return (energyLoss[0]*1e-03)/energy
        return((E_final)/energy)
    elif (diff < range_data[0] and diff >= 0):
        '''
        interp_function = interpolate.interp1d(range_data[:energy_pos[0] + 1], de_dx[:energy_pos[0] + 1], kind='cubic')
        energyLoss = integrate.quad(interp_function,range_data[0],range_data[energy_pos[0]])
        return (energyLoss[0]*1e-03)/energy
        '''
    else:
        return 0

def find_nearest(array, value):
    array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return array[idx]

def line_filter(line):
    line1 = line.replace(" keV", "E-03")
    line2 = line1.replace(" MeV", "")
    line3 = line2.replace(" A", "E-04")
    line4 = line3.replace(" um", "")
    line5 = line4.replace(" mm", "E03")
    line6 = line5.replace("   ", " ")
    line7 = line6.replace("  "," ")
    line8 = line7.replace("  "," ")
    line9 = line8.replace(",",".")
    line_final = line9[1:]
    #print(line_final)
    return line_final


def get_thick_line(filename,value): # get line with data from 50um from SRIM table
    file = open("specialValues/{0}".format(filename), "r") 
    value_find = str(value)
    for line in file:
        if re.search(value_find,line):
            line_format = line_filter(line)
            return line_format
            

def format_file(filename,element): # Format SRIM tables and return lists
    file = open("tables/{0}".format(filename), "r") 
    lines_data = []
    for i, line in enumerate(file):
        if i in np.arange(25, 130): 
            lines_data.append(line_filter(line)) 
    # After getting a list of lines of data, get the line of 15um, add it to list and sort it.
    line_special = get_thick_line(filename,thickness)
    repeated_line = False
    for j in lines_data:
        if j == line_special:
            repeated_line = True
        
    if not repeated_line:
        lines_data.append( line_special)

    #lines_data.append(line_format_15)
    data_array_unsort = np.loadtxt(lines_data)
    #sort numpy array by the first collumn
    data_array = data_array_unsort[np.argsort(data_array_unsort[:, 0])]
    return data_array

print("---------------" + radical[:-4] + "-----------------------")
print("Thickness =", thickness)

for element in elements: #Loop in all files of folder
    filename = "{0}_{1}".format(element, radical)
    data_arr = format_file(filename,element)
    dE_dx = np.add(data_arr[:,1],data_arr[:,2]) # Get sum of colluns 1 e 2 (of dE/dx)
    range_data = data_arr[:,3] #Get range collumn
    long_strag = data_arr[:,4] #Get longitudinal straggling
    kinEn = data_arr[:,0] # Get kinetic energy(initial) 
  
    #-------- interpolation -------------


    # fist, we need to find the position of range_data 15 
    value_near = find_nearest(range_data, thickness)
    print(value_near)
    pos, = np.where(range_data == value_near)

    # Now we find the nearest point of range_data in tables giving the longitudinal
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

    

    print('---------------------{0}-----------------'.format(element))
    print('Longitudinal straggling at {0}um and {2}: dx = {1}'.format(value_near,long_strag[pos],kinEn[pos]))
    print('Correspondent kinEn for range_data of {0} um: E = {1} MeV'.format(long_strag[pos],energy_straggling))
    print('is interpolation necessary: {0}'.format(need_interpol))
    print("nearest point: {0} um".format(near_point))
    print('range_data interpolation: {0} MeV'.format(kinEn_interp))

    if(EnergyLossTable):
        print("------------------------------------------------------------------------")
        print("Fraction of energy loss due to the ALPIDE")
        for i in initial_energy:
            fracEn = calculate_energyLossFraction(dE_dx,range_data,kinEn,i)
            print("{0} MeV --------------------- {1:.1f}".format( i,fracEn*100))
        print("------------------------------------------------------------------------")
    '''
    plt.xlabel("range_data($\mu$m)")
    plt.ylabel("Kinetic Energy (MeV)")
    plt.plot(range_data_graph,kinEn_graph, 'o',label="SRIM-2013")
    plt.plot(range_data_interp,interp_function(range_interp),'-',label='linear interpolation')
    plt.plot(long_strag[pos], energy_straggling, 'o', color='r', label='KinEn = {0}; range_data = {1}'.format(energy_straggling,long_strag[pos]))
    plt.title("Hydrogen")
    plt.legend()
    plt.show()
    '''   
