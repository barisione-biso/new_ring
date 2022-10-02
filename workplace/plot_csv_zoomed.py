#Started as a copy of automated_test.py
import os
import csv

import pandas as pd
import matplotlib.pyplot as plt

import argparse

# Initialize parser
parser = argparse.ArgumentParser()
 
# Adding optional argument
parser.add_argument("-f", "--First", help = "First csv in the plot.", required=True)
parser.add_argument("-s", "--Second", help = "Second csv in the plot.", required=True)
parser.add_argument("-t", "--Third", help = "Third csv in the plot.", required=True)
parser.add_argument("-fo", "--Fourth", help = "Fourth csv in the plot.", required=True)
parser.add_argument("-fi", "--Fifth", help = "Fifth csv in the plot.", required=True)
parser.add_argument("-o", "--OutputFolder", help = "Output folder.")

# Read arguments from command line
args = parser.parse_args()
'''
output_folder = "."
if args.OutputFolder:
    output_folder = args.OutputFolder
'''

lists_of_rows = [] #A list of lists.
files = [ args.First, args.Second, args.Third, args.Fourth, args.Fifth]

print("****** Plotting Starts.")
#SECOND PART
success=True
for file in files:
    #Opening sigmod file.
    with open(file, 'r') as csvfile:
        mode_rows = []
        # creating a csv reader object
        csvreader = csv.reader(csvfile)

        # extracting each data row one by one
        for row in csvreader:
            mode_rows.append(row)
        lists_of_rows.append(mode_rows)
        # get total number of rows
        print("Total # of "+file+" rows: %d"%(csvreader.line_num))
        if csvreader.line_num == 0:
            success = False
if not success:
    print("An error has occured. Stoping the automated testing.")
    exit()

print("****** Checking for result correctness.")
#TODO: FROM HERE ONWARDS CODE GENERALIZATION IS NEEDED. rows has to be renamed
#rows[0] : SIGMOD21, rows[1]: muthu
num_of_results_error = 0
sigmod21_performance = []
one_ring_muthu_leap_performance = []
one_ring_muthu_adaptive_leap_performance = []
sigmod21_adaptive_performance = []
backward_only_performance = []
for index, sigmod_row in enumerate(lists_of_rows[0]):
    # working with the second column of the row
    aux=sigmod_row[0].split(";")
    aux2=lists_of_rows[1][index][0].split(";")
    aux3=lists_of_rows[2][index][0].split(";")
    aux4=lists_of_rows[3][index][0].split(";")
    aux5=lists_of_rows[4][index][0].split(";")
    if aux[1] != aux2[1] or aux[1] != aux3[1] or aux[1] != aux4[1] or aux[1] != aux5[1]:
        num_of_results_error=num_of_results_error+1

    sigmod21_performance.append(int(aux[2]))
    one_ring_muthu_leap_performance.append(int(aux2[2]))
    one_ring_muthu_adaptive_leap_performance.append(int(aux3[2]))
    sigmod21_adaptive_performance.append(int(aux4[2]))
    backward_only_performance.append(int(aux5[2]))

print("****** Number of different results: ", num_of_results_error)

#THIRD PART. Producing the input files for the sigmod21 plotting mechanism in overleaf.
print("****** Generating input files for sigmod21 plotting mechanism (overleaf).")
for file in files:
    command='../plots/box-plot-sigmod21/generate_output '+file+' ../plots/box-plot-sigmod21/query_types.txt'
    print("Running command: "+command)
    os.system(command)

#FOURTH PART
print("****** Plotting variants using matplotlib.")

d = {'Orig': sigmod21_performance,
    'Orig Adaptive': sigmod21_adaptive_performance,
    'Muthu': one_ring_muthu_leap_performance,
    'Muthu adaptive': one_ring_muthu_adaptive_leap_performance,
    'Backward' : backward_only_performance}

df = pd.DataFrame(data = d)
plt.ylim(10000, 15000000)
df.boxplot()

#plt.show()
plt.savefig('plot_zoomed.pdf', format='pdf')
#https://stackoverflow.com/questions/69828508/warning-ignoring-xdg-session-type-wayland-on-gnome-use-qt-qpa-platform-wayland


print("****** Plotting Ends.")
