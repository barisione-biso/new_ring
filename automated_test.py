import os
import csv

import pandas as pd
import matplotlib.pyplot as plt

import argparse

# Initialize parser
parser = argparse.ArgumentParser()
 
# Adding optional argument
parser.add_argument("-i", "--Index", help = "Index file [.ring | .cring].", required=True)
parser.add_argument("-q", "--Queries", help = "Queries file.", required=True)
parser.add_argument("-r", "--Results", help = "Number of results.")
parser.add_argument("-t", "--Timeout", help = "Query Timeout.")
parser.add_argument("-o", "--OutputFolder", help = "Output folder.")
# Read arguments from command line
args = parser.parse_args()

#FIRST PART.
lists_of_rows = [] #A list of lists.
#Wikidata filtered enumerated automated test.
#dataset="wikidata-filtered-enumerated.dat.ring" #wikidata.nt.enumerated.ring
#queries="Queries-wikidata-benchmark.txt"
dataset_full_path=args.Index
dataset = os.path.basename(dataset_full_path)
queries_full_path=args.Queries
queries = os.path.basename(queries_full_path)
number_of_results = "1000"
timeout = "600"
if args.Results:
    number_of_results = args.Results
if args.Timeout:
    timeout = args.Timeout
output_folder = "."
if args.OutputFolder:
    output_folder = args.OutputFolder

print("****** Starting automated test.")
print("Index file: ", dataset, " located at :", os.path.dirname(dataset_full_path))
print("Query file: ", queries, " located at :", os.path.dirname(queries_full_path))
print("Output folder: ", output_folder)
available_modes = ["sigmod21", "one_ring_muthu_leap", "one_ring_muthu_leap_adaptive"]

print("Available modes : "+",".join(available_modes))
for mode in available_modes:
    print("Running queries for dataset '"+dataset+"' using '"+mode+"' mode.")
    cmd = './build/query-index ../data/'+dataset+' Queries/'+queries+' '+mode+' 0 0 '+number_of_results+' ' + timeout + ' > '+output_folder+'/tmp_'+mode+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv'
    print(cmd)
    os.system(cmd)

#SECOND PART
success=True
for mode_idx, mode in enumerate(available_modes):
    #Opening sigmod file.
    with open(output_folder+'/tmp_'+mode+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv', 'r') as csvfile:
        mode_rows = []
        # creating a csv reader object
        csvreader = csv.reader(csvfile)

        # extracting each data row one by one
        for row in csvreader:
            mode_rows.append(row)
        lists_of_rows.append(mode_rows)
        # get total number of rows
        print("Total # of "+mode+" rows: %d"%(csvreader.line_num))
        if csvreader.line_num == 0:
            success = False
if not success:
    print("An error has occured. Stoping the automated testing.")
    exit()

print("****** Checking for result correctness.")
#TODO: FROM HERE ONWARDS CODE GENERALIZATION IS NEEDED. rows has to be renamed
#rows[0] : SIGMOD21, rows[1]: muthu
num_of_results_error = 0
sigmod_performance = []
one_ring_muthu_leap_performance = []
one_ring_muthu_adaptative_leap_performance = []
for index, sigmod_row in enumerate(lists_of_rows[0]):
    # working with the second column of the row
    aux=sigmod_row[0].split(";")
    aux2=lists_of_rows[1][index][0].split(";")
    aux3=lists_of_rows[2][index][0].split(";")
    if aux[1] != aux2[1] and aux[1] != aux3[1]:
        num_of_results_error=num_of_results_error+1

    sigmod_performance.append(int(aux[2]))
    one_ring_muthu_leap_performance.append(int(aux2[2]))
    one_ring_muthu_adaptative_leap_performance.append(int(aux3[2]))

print("****** Number of different results: ", num_of_results_error)

#THIRD PART. Producing the input files for the sigmod21 plotting mechanism in overleaf.
print("****** Generating input files for sigmod21 plotting mechanism (overleaf).")
for mode in available_modes:
    command='./plots/box-plot-sigmod21/generate_output '+output_folder+'/tmp_'+mode+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv plots/box-plot-sigmod21/query_types.txt'
    print("Running command: "+command)
    os.system(command)

#FOURTH PART
print("****** Plotting variants using matplotlib.")

d = {'Sigmod21': sigmod_performance,
    '1 Ring Muthu': one_ring_muthu_leap_performance,
    '1 Ring Muthu adaptive': one_ring_muthu_adaptative_leap_performance}

df = pd.DataFrame(data = d)
plt.ylim(10000, 100000000)
df.boxplot()

plt.show()
#plot.get_figure().savefig('plot.pdf', format='pdf')
#https://stackoverflow.com/questions/69828508/warning-ignoring-xdg-session-type-wayland-on-gnome-use-qt-qpa-platform-wayland


print("****** Ending automated test.")