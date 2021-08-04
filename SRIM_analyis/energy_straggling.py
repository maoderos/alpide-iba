import os
import sys
import re
import numpy as np
from io import StringIO 

#Function to get line with 15um

def get_15um_line(file): # get line with data from 15um from SRIM table
    for line in file:
        if re.search("15,00 um",line):
            return line
        elif re.search("15,01 um",line):
            return line 

def format_file(file,element): # Format SRIM tables and return lists
    kinEn =[]; dE_dx = []; range = []; long_strag =[]; lat_strag = [];
    for i, line in enumerate(file):
        if i in np.arange(25, 104):
            line1 = line.replace(" keV", "e-03")
            line2 = line1.replace(" MeV", "")
            line3 = line2.replace(" A", "e-04")
            line4 = line3.replace(" um", "")
            line5 = line4.replace("   ", " ")
            line6 = line5.replace("  "," ")
            line7 = line6.replace("  "," ")
            line8 = line7.replace(",",".")
            line_final = StringIO(line8)
            # use numpy to separate formated line and append to lists above
            E, dE_dx_elec, dE_dx_nuc, proj_rang, long_straggling, lat_straggling = np.loadtxt(line_final, unpack=True)
            kinEn.append(E); range.append(proj_rang); long_strag.append(long_straggling); lat_strag.append(lat_straggling)
            dE_dx_sum = np.add(dE_dx_elec,dE_dx_nuc) #sum the stopping powers
            dE_dx.append(dE_dx_sum)
    return kinEn, dE_dx, range, long_strag, lat_strag

#elements = ["H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne"]
elements = ["H"]
radical = "Al-Si-O.txt"


for element in elements: #Loop in all files of folder
    filename = "{0}_{1}".format(element, radical)
    generic_file = open("tables/{0}".format(filename), "r")
    kinEn, dE_dx, range, long_strag, lat_strag = format_file(generic_file, element) # format file and tranform data indo lists
    # TODO: Test interpolation to see if 15 file is necessary 
    # Study interpolation in python and types
    # If yes -> Integral for 15 + 3sigma and 15 - 3sigma -> Get Sigma from file (?)
    # If no -> Get values, insert into list, interpolate to straggling and integrate 

