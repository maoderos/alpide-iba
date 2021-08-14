# TODO: Test interpolation to see if 15um file is necessary 
# Correct the sort problem (DONE)
# Study interpolation in python and types
# If yes -> Integral for 15 + sigma and 15 - sigma -> Get Sigma from file (?)
# If no -> Get values, insert into list, interpolate to straggling and integrate 

import os
import sys
import re
import numpy as np
from io import StringIO 
import matplotlib.pyplot as plt
import scipy.interpolate as interpolate
import scipy.integrate as integrate

metal_stack_thick = 15.00 

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
    return line_final


def get_15um_line(filename): # get line with data from 15um from SRIM table
    file = open("15um_values/{0}".format(filename), "r") 
    for line in file:
        if re.search("15,00 um",line) or re.search("15,01 um",line):
            line_format = line_filter(line)
            return line_format
            

def format_file(filename,element): # Format SRIM tables and return lists
    kinEn =[]; dE_dx = []; range = []; long_strag =[]; lat_strag = [];
    file = open("tables/{0}".format(filename), "r") 
    lines_data = []
    for i, line in enumerate(file):
        if i in np.arange(25, 130): 
            lines_data.append(line_filter(line)) 
    # After getting a list of lines of data, get the line of 15um, add it to list and sort it.
    lines_data.append(get_15um_line(filename))
    #lines_data.append(line_format_15)
    data_array_unsort = np.loadtxt(lines_data)
    #sort numpy array by the first collumn
    data_array = data_array_unsort[np.argsort(data_array_unsort[:, 0])]
    
    return data_array


elements = ["H", "He", "Li" ,"Be", "B", "C", "N", "O", "F", "Ne"]
#elements = ["H"]
radical = "Al-Si-O.txt"


for element in elements: #Loop in all files of folder
    filename = "{0}_{1}".format(element, radical)
    data_arr = format_file(filename,element)
    dE_dx = np.add(data_arr[:,1],data_arr[:,2]) # Get sum of colluns 1 e 2 (of dE/dx)
    range = data_arr[:,3] #Get range collumn
    long_strag = data_arr[:,4] #Get longitudinal straggling
    kinEn = data_arr[:,0] # Get kinetic energy(initial) 
  
    #-------- interpolation -------------

    # fist, we need to find the position of range 15 or 15.01 in array
    pos_1500, = np.where(range == 15.00)
    pos_1501, = np.where(range == 15.01)
    if pos_1500.size != 0:
        pos_15, = np.where(range == 15.00)
    elif pos_1501.size != 0:
        pos_15, = np.where(range == 15.01)
    
    #interpolate 3 adjacent points between 15um
    #range_interp = range[pos_15[0] - 4:pos_15[0] + 5]
    #dE_dx_interp = dE_dx[pos_15[0] - 4:pos_15[0] + 5]

    # Now we find the nearest point of range in tables giving the longitudinal
    # straggling of 15um

    near_point = find_nearest(range, long_strag[pos_15])

    need_interpol = True
    near_p_idx, = np.where(range == near_point)
    if near_point > long_strag[pos_15]:
        range_interp = range[near_p_idx[0]-1:near_p_idx[0] + 1]
        kinEn_interp = kinEn[near_p_idx[0]-1:near_p_idx[0] + 1]
    elif near_point < long_strag[pos_15]:
        range_interp = range[near_p_idx[0]:near_p_idx[0] + 2]
        kinEn_interp = kinEn[near_p_idx[0]:near_p_idx[0] + 2]
    elif near_point == long_strag[pos_15]:
        need_interpol = False
        energy_straggling_array, = kinEn[np.where(range == long_strag[pos_15])]
        energy_straggling = energy_straggling_array[0]
    
    # if the nearest point doesnt appear in table, intepolate to estimate kinEn

    if (need_interpol):
        interp_function = interpolate.interp1d(range_interp, kinEn_interp, kind='linear')
        energy_straggling = interp_function(long_strag[pos_15])


    print('---------------------{0}-----------------'.format(element))
    print('Longitudinal straggling at 15um: dx = {0}'.format(long_strag[pos_15]))
    print('Correspondent kinEn for range of {0} um: E = {1} MeV'.format(long_strag[pos_15],energy_straggling))
    print('is interpolation necessary: {0}'.format(need_interpol))
    print("nearest point: {0} um".format(near_point))
    print('range interpolation: {0} MeV'.format(kinEn_interp))

    #plt.xlabel("range($\mu$m)")
    #plt.ylabel("Kinetic Energy (MeV)")
    #plt.plot()
    #plt.plot(range_interp,kinEn_interp, 'o',label="SRIM-2013")
    #plt.plot(range_interp,interp_function(range_interp),'-',label='linear interpolation')
    #plt.plot(long_strag[pos_15], energy_straggling, 'o', color='r', label='stragg_point')
    #plt.legend()
    #plt.show()