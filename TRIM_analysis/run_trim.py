from srim import Ion, Layer, Target, TRIM
import os
import sys

n_ions = 2e4
ion_name = str(sys.argv[1])
energy_ion = float(sys.argv[2])*1e6

# Construct a 3MeV Nickel ion
ion = Ion(ion_name, energy=energy_ion)

# Construct a layer of metal 10um thick 
metal = Layer({
        'Al': {'stoich': 0.17,
               'E_d': 25.0, 
               'lattice': 3.0, 
               'surface': 3.36}, 
        'Si': {'stoich': 0.388,
               'E_d': 15.0, 
               'lattice': 2.0, 
               'surface': 4.7}, 
        'O' : {'stoich': 0.442,
               'E_d': 28.0, 
               'lattice': 3.0, 
               'surface': 2.0}
        }, density=2.28, width=100000.0) 
        
si = Layer({'Si': {'stoich': 1.0,
               'E_d': 15.0, 
               'lattice': 2.0, 
               'surface': 4.7}}, density=2.3212,width=400000.0) # Layer of Silicon with 40um thick

# Construct a target of a single layer of Nickel
target = Target([metal,si])

# Initialize a TRIM with n number of ions and Full Cascade (2)
trim = TRIM(target, ion, number_ions=n_ions, calculation=2)

# Specify the directory of SRIM.exe
srim_executable_directory = '/home/marcosderos/.wine/drive_c/Program Files (x86)/SRIM'


results = trim.run(srim_executable_directory)

drt = os.getcwd()
TRIM.copy_output_files('/home/marcosderos/.wine/drive_c/Program Files (x86)/SRIM', drt)

print("SIMULATION OF {0} IONS OF {1} WITH {2} MeV IN ALPIDE DONE.".format(n_ions, ion_name, sys.argv[2]))




