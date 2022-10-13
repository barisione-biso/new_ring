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
parser.add_argument("-si", "--Sixth", help = "Sixth csv in the plot.", required=True)
parser.add_argument("-se", "--Seventh", help = "Seventh csv in the plot.", required=True)
parser.add_argument("-ei", "--Eighth", help = "Eighth csv in the plot.", required=True)
parser.add_argument("-ni", "--Ninth", help = "Ninth csv in the plot.", required=True)
parser.add_argument("-te", "--Tenth", help = "Tenth csv in the plot.", required=True)
parser.add_argument("-o", "--OutputFolder", help = "Output folder.")

# Read arguments from command line
args = parser.parse_args()
'''
output_folder = "."
if args.OutputFolder:
    output_folder = args.OutputFolder
'''

lists_of_rows = [] #A list of lists.
files = [ args.First, args.Second, args.Third, args.Fourth, args.Fifth, args.Sixth, args.Seventh, args.Eighth, args.Ninth, args.Tenth]

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
first_arg_performance = []
second_arg_performance = []
third_arg_performance = []
fourth_arg_performance = []
fifth_arg_performance = []
sixth_arg_performance = []
seventh_arg_performance = []
eighth_arg_performance = []
ninth_arg_performance = []
tenth_arg_performance = []
for index, sigmod_row in enumerate(lists_of_rows[0]):
    # working with the second column of the row
    aux1=sigmod_row[0].split(";")
    aux2=lists_of_rows[1][index][0].split(";")
    aux3=lists_of_rows[2][index][0].split(";")
    aux4=lists_of_rows[3][index][0].split(";")
    aux5=lists_of_rows[4][index][0].split(";")
    aux6=lists_of_rows[5][index][0].split(";")
    aux7=lists_of_rows[6][index][0].split(";")
    aux8=lists_of_rows[7][index][0].split(";")
    aux9=lists_of_rows[8][index][0].split(";")
    aux10=lists_of_rows[9][index][0].split(";")
    if aux1[1] != aux2[1] or aux1[1] != aux3[1] or aux1[1] != aux4[1] or aux1[1] != aux5[1] or aux1[1] != aux6[1] or aux1[1] != aux7[1] or aux1[1] != aux8[1] or aux1[1] != aux9[1] or aux1[1] != aux10[1]:
        num_of_results_error=num_of_results_error+1

    first_arg_performance.append(float(aux1[2])/1000000000)
    second_arg_performance.append(float(aux2[2])/1000000000)
    third_arg_performance.append(float(aux3[2])/1000000000)
    fourth_arg_performance.append(float(aux4[2])/1000000000)
    fifth_arg_performance.append(float(aux5[2])/1000000000)
    sixth_arg_performance.append(float(aux6[2])/1000000000)
    seventh_arg_performance.append(float(aux7[2])/1000000000)
    eighth_arg_performance.append(float(aux8[2])/1000000000)
    ninth_arg_performance.append(float(aux9[2])/1000000000)
    tenth_arg_performance.append(float(aux10[2])/1000000000)

print("****** Number of different results: ", num_of_results_error)

#THIRD PART. Producing the input files for the sigmod21 plotting mechanism in overleaf.
print("****** Generating input files for sigmod21 plotting mechanism (overleaf).")
for file in files:
    command='../plots/box-plot-sigmod21/generate_output '+file+' ../plots/box-plot-sigmod21/query_types.txt'
    print("Running command: "+command)
    os.system(command)

#FOURTH PART
print("****** Plotting variants using matplotlib.")

d = {'Orig': first_arg_performance,
    'Muthu': second_arg_performance,
    'Muthu adaptive': third_arg_performance,
    'Backward' : fourth_arg_performance,
    'Backward Muthu': fifth_arg_performance,
    'Backward Adaptive': sixth_arg_performance,
    'Backward Adaptive Muthu': seventh_arg_performance,
    'Backward (leap)': eighth_arg_performance,
    'Backward (leap) Muthu' : ninth_arg_performance,
    'Backward (leap) adaptive Muthu' : tenth_arg_performance
    }

df = pd.DataFrame(data = d)
plt.ylim(0, 0.01)
df.boxplot()

#plt.show()
plt.savefig('plot_zoomed.pdf', format='pdf')
#https://stackoverflow.com/questions/69828508/warning-ignoring-xdg-session-type-wayland-on-gnome-use-qt-qpa-platform-wayland


print("****** Plotting Ends.")
