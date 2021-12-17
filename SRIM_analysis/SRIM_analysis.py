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
density_alpide = 2.313 # g/cm3
density_metal = 2.28 # g/cm3 
density_si = 2.32 # g/cme
radiation_length = [21.82, 24.01, 34.24] # Si,Al,O
weigth_alpide =  [0.8776, 0.034, 0.0884] # Si, Al, O
weigth_metal = [0.388, 0.17, 0.442] 
weigth_si = [1.0]

table_totalEloss = []
table_elecEloss = []
table_nucEloss = []
table_multScattEloss = []

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
            

def format_file(filename,element,get_thick=True,init=25,end=130): # Format SRIM tables and return lists
    file = open("tables/{0}".format(filename), "r") 
    lines_data = []
    for i, line in enumerate(file):
        if i in np.arange(init, end): 
            lines_data.append(line_filter(line)) 
    # After getting a list of lines of data, get the line of thickness, add it to list and sort it.
    if(get_thick):
    
        line_special = get_thick_line(filename,thickness)
        repeated_line = False
        for j in lines_data:
            if j == line_special:
                repeated_line = True
        
        if not repeated_line:
            lines_data.append(line_special)
    #print(lines_data)
    #lines_data.append(line_format_15)
    data_array_unsort = np.loadtxt(lines_data)
    #sort numpy array by the first collumn
    data_array = data_array_unsort[np.argsort(data_array_unsort[:, 0])]
    return data_array


def gen_latex_table(table,energies):
    print(r"\begin{table}[]" + "\n" + r"\resizebox{10.0cm}{!}{" + "\n" + r"\begin{tabular}{|l|c|c|c|c|c|c|c|c|c|c|c|c|c|}" + "\n\hline\n")
    header = "Element"
    for i in energies:
        header += r" & ${0} \ MeV$".format(i)
    print(header + r" \\" )
    print(r"\hline")
    for j in table:
        j = j.replace("$1000.0^{\circ}$", "-")
        j = j.replace("$-100.0\%$","-")
        j = j.replace("$-1.0E+07$","-")
        j = j.replace("$-10000.0$", "-")
        print(j)
    print("\hline\n \end{tabular} \n } \n \end{table}")


print("---------------" + radical[:-4] + "-----------------------")
print("Thickness =", thickness)

element_z = 1 #starting at H 
for element in elements: #Loop in all files of folder
    filename = "{0}_alpide.txt".format(element)
    filename_si = "{0}-Si.txt".format(element)
    filename_metal = "{0}_Al-Si-O.txt".format(element)
    data_arr = format_file(filename,element)
    data_si = format_file(filename_si,element,False,23,128)
    data_metal = format_file(filename_metal,element,False)
    dE_dx = np.add(data_arr[:,1],data_arr[:,2]) # Get sum of colluns 1 e 2 (of dE/dx)
    dE_dx_elec = data_arr[:,1]
    dE_dx_nuc = data_arr[:,2]
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
        line_total = element
        line_elec = element
        line_nuc = element
        line_multScatt = element
        for i in initial_energy:
            En_loss_total,En_loss_elec,En_loss_nuc,theta_slice = calculate_energyLossFraction(data_metal,data_si,i,thickness,element_mass[element_z - 1],radiation_length,weigth_metal, weigth_si,density_metal,density_si)
          
            line_total += " & ${0:.1f}\%$".format((En_loss_total/i)*100)
            line_elec += " & ${0:.1E}$".format(En_loss_elec*1e6)
            line_nuc += " & ${0:.1E}$".format(En_loss_nuc*1e6) # to eV
            line_multScatt += " & ${0:.1f}".format(theta_slice)
            line_multScatt += "^{\circ}$"
            #print("{0} MeV --------------------- {1:.1f} ----------------{2:.1f}------------------- {3:.1f}".format(i,fracEn*100, theta_scat ,theta_slice))
        
        print("------------------------------------------------------------------------")
        line_total += r" \\"
        line_elec += r" \\"
        line_nuc += r" \\"
        line_multScatt += r" \\"
        table_totalEloss.append(line_total)
        table_elecEloss.append(line_elec)
        table_nucEloss.append(line_nuc)
        table_multScattEloss.append(line_multScatt)
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

print("------------------Tables-------------------")
print("------------------Total Energy-------------------") 
gen_latex_table(table_totalEloss,initial_energy)
print("------------------Electronic Energy-------------------")
gen_latex_table(table_elecEloss,initial_energy)
print("------------------Nuclear Energy( keV )----------------------")
gen_latex_table(table_nucEloss,initial_energy)
print("------------------Multiple Scattering----------------------")
gen_latex_table(table_multScattEloss,initial_energy)
print("End of Program")
