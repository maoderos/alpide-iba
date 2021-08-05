# TODO: Test interpolation to see if 15um file is necessary 
# Correct the sort problem (DONE)
# Study interpolation in python and types
# If yes -> Integral for 15 + 3sigma and 15 - 3sigma -> Get Sigma from file (?)
# If no -> Get values, insert into list, interpolate to straggling and integrate 


import os
import sys
import re
import numpy as np
from io import StringIO 
import matplotlib.pyplot as plt

#Function to get line with 15um

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
        if i in np.arange(25, 104):
            lines_data.append(line_filter(line)) 
    # After getting a list of lines of data, get the line of 15um, add it to list and sort it.
    lines_data.append(get_15um_line(filename))
    #lines_data.append(line_format_15)
    data_array_unsort = np.loadtxt(lines_data)
    #sort numpy array by the first collumn
    data_array = data_array_unsort[np.argsort(data_array_unsort[:, 0])]
    
    return data_array


elements = ["H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne"]
#elements = ["H"]
radical = "Al-Si-O.txt"


for element in elements: #Loop in all files of folder
    filename = "{0}_{1}".format(element, radical)
    #kinEn, dE_dx, range, long_strag, lat_strag = format_file(filename, element) # format file and tranform data indo lists
    data_arr = format_file(filename,element)
    #kinEn_15, dE_dx_15, long_strag_15, lat_strag_15 = get_15um_line(filename)


    #plt.xlabel("range(um)")
    #plt.ylabel("dE/dx (keV/um)")
    #plt.xlim(0,20)
    #plt.plot(range,dE_dx)
    #plt.show()



