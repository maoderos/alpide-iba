import os
import sys
import re
import numpy as np
from io import StringIO 
import matplotlib.pyplot as plt
import scipy.interpolate as interpolate
from energyLoss_complexScatt import *
from energyStragg import *
from multipleScatt import *



#--------Initial values----------

elements = ["H", "He", "Li" ,"Be", "B", "C", "N", "O", "F", "Ne"]
element_mass = [1.0079, 4.0026, 6.941, 9.0122, 10.811, 12.0107, 14.0067, 15.9994, 18.9984, 20.1797]
mev_c = 931.4940
element_mass = [x*mev_c for x in element_mass] # conversion of atomic mass in MeV/c^2
#elements = ["H"]
initial_energy = [4.0,8.0,10.0,15.0,20.0,30.0,40.0,50.0,60.0,70.0,80.0,90.0,100.0] # MeV
radical = "alpide.txt"
EnergyLossTable = True

#----ALPIDE parameters and materials
thickness = 50.0 #in um
density = 2.313 # g/cm3
radiation_length = [21.82, 24.01, 34.24] # Si,Al,O
weigth =  [0.8776, 0.034, 0.0884] # Si, Al, O

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


def get_thick_line(filename,value): # get line with data of thickness from SRIM table
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
    # After getting a list of lines of data, get the line of thickness, add it to list and sort it.
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

element_z = 1 #starting at H 
for element in elements: #Loop in all files of folder
    filename = "{0}_{1}".format(element, radical)
    data_arr = format_file(filename,element)
    dE_dx = np.add(data_arr[:,1],data_arr[:,2]) # Get sum of colluns 1 e 2 (of dE/dx)
    range_data = data_arr[:,3] #Get range collumn
    long_strag = data_arr[:,4] #Get longitudinal straggling
    kinEn = data_arr[:,0] # Get kinetic energy(initial) 
  
    #-------- Getting energy Straggling ------------
    
    energy_straggling, pos, need_interpol, near_point, value_near = getEnergyStraggling(thickness, range_data, long_strag, kinEn)

    

    print('---------------------{0}-----------------'.format(element))
    print('Longitudinal straggling at {0}um and {2}: dx = {1}'.format(value_near,long_strag[pos],kinEn[pos]))
    print('Correspondent kinEn for range_data of {0} um: E = {1} MeV'.format(long_strag[pos],energy_straggling))
    print('is interpolation necessary: {0}'.format(need_interpol))
    print("nearest point: {0} um".format(near_point))
   # print('range_data interpolation: {0} MeV'.format(kinEn_interp))

    if(EnergyLossTable):
        print("------------------------------------------------------------------------")
        print("Fraction of energy loss due to the ALPIDE")

        print("Initial Energy ------------------ FracEnLoss -------------------- Scatt_Angle1-------------------Scatt_AngleSlice")
        for i in initial_energy:
            fracEn, theta_slice = calculate_energyLossFraction(dE_dx,range_data,kinEn,i,thickness,element_mass[element_z - 1],radiation_length,weigth, density)
            if (fracEn != 0):
                theta_scat = getMultipleScatt(i,fracEn,element_mass[element_z - 1],1,thickness, radiation_length, weigth, density)
            else:
                theta_scat = 100000
            print("{0} MeV --------------------- {1:.1f} ----------------{2:.1f}------------------- {3:.1f}".format(i,fracEn*100, theta_scat ,theta_slice))
        print("------------------------------------------------------------------------")
    element_z += 1

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
print("End of Program")
