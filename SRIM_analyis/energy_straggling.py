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

def line_filter(line):
    line1 = line.replace(" keV", "E-03")
    line2 = line1.replace(" MeV", "")
    line3 = line2.replace(" A", "E-04")
    line4 = line3.replace(" um", "")
    line5 = line4.replace("   ", " ")
    line6 = line5.replace("  "," ")
    line7 = line6.replace("  "," ")
    line8 = line7.replace(",",".")
    line_final = line8[1:]
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
        if i in np.arange(25, 104): #130
            lines_data.append(line_filter(line)) 
    # After getting a list of lines of data, get the line of 15um, add it to list and sort it.
    lines_data.append(get_15um_line(filename))
    #lines_data.append(line_format_15)
    data_array_unsort = np.loadtxt(lines_data)
    #sort numpy array by the first collumn
    data_array = data_array_unsort[np.argsort(data_array_unsort[:, 0])]
    
    return data_array


#elements = ["H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne"]
elements = ["H"]
radical = "Al-Si-O.txt"


for element in elements: #Loop in all files of folder
    filename = "{0}_{1}".format(element, radical)
    data_arr = format_file(filename,element)
    dE_dx = np.add(data_arr[:,1],data_arr[:,2]) # Get sum of colluns 1 e 2 (of dE/dx)
    range = data_arr[:,3] #Get range collumn
    long_strag = data_arr[:,4] #Get longitudinal straggling
  
    #-------- interpolation -------------

    # fist, we need to find the position of range 15 or 15.01 in array
    pos_1500, = np.where(range == 15.00)
    pos_1501, = np.where(range == 15.01)
    if pos_1500.size != 0:
        pos_15, = np.where(range == 15.00)
    elif pos_1501.size != 0:
        pos_15, = np.where(range == 15.01)
    
    #interpolate 3 adjacent points between 15um
    range_interp = range[pos_15[0] - 4:pos_15[0] + 5]
    dE_dx_interp = dE_dx[pos_15[0] - 4:pos_15[0] + 5]

    print(range_interp)
    print(dE_dx_interp)

    interp_function = interpolate.interp1d(range_interp, dE_dx_interp, kind='linear')
    
    sigma15_plus = 15.00 + long_strag[pos_15]
    sigma15_minus = 15.00 - long_strag[pos_15]

    dE_dx_15_sigma_plus = interp_function(sigma15_plus)
    dE_dx_15_sigma_minus = interp_function(sigma15_minus) 

    # Creating a sample with data for integration
    
    inty_plus = [dE_dx[pos_15][0],dE_dx_15_sigma_plus[0]]
    
    inty_minus = [dE_dx_15_sigma_minus[0],dE_dx[pos_15][0]]
    intx_plus = [range[pos_15][0],sigma15_plus[0]]
    intx_minus = [sigma15_minus[0], range[pos_15][0]]

    ## ---- Integration -----
    Delta_plus_simpson = integrate.simps(inty_plus,intx_plus)
    Delta_plus_trapezoid = integrate.trapezoid(inty_plus,intx_plus)
    Delta_minus_simpson = integrate.simps(inty_minus,intx_minus)
    Delta_minus_trapezoid = integrate.trapezoid(inty_minus,intx_minus)
 
    print("--------------{0}--------------\n".format(element))
    print("Simpson's method:\n")
    print("Delta+ = {0} keV | Delta- = {1} keV\n".format(Delta_plus_simpson,Delta_minus_simpson))
    print("Trapezoid method:\n")
    print("Delta+ = {0} keV | Delta- = {1} keV\n".format(Delta_plus_trapezoid,Delta_minus_trapezoid))
    print("----------------------------\n")
    print("using intepolation linear function (with 15um point)")
    Delta_plus_int = integrate.quad(interp_function, range[pos_15][0], sigma15_plus[0])
    Delta_minus_int = integrate.quad(interp_function, sigma15_minus[0],range[pos_15][0])
    print("Delta+ interpolation = {0}".format(Delta_plus_int))
    print("Delta- interpolation = {0}".format(Delta_minus_int))
    #plt.xlabel("range($\mu$m)")
    #plt.ylabel("dE/dx (keV/$\mu$m)")
    #plt.plot()
    #plt.plot(range_interp,dE_dx_interp, 'o',label="SRIM-2013")
    #plt.plot(range_interp,interp_function(range_interp),'-',label='linear interpolation')
    #plt.plot((15 + long_strag[pos_15]),dE_dx_15_sigma_plus, 'o',color='r',label='15$\mu \pm \sigma$' )
    #plt.plot((15 - long_strag[pos_15]),dE_dx_15_sigma_minus, 'o',color='r')
    #plt.legend()
    #plt.show()
    #plt.savefig("interpolation.pdf")