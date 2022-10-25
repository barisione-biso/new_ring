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
available_variants = ["sigmod21", "sigmod21_adaptive", "one_ring_muthu_leap", "one_ring_muthu_leap_adaptive", "backward_only", "backward_only_muthu", "backward_only_adaptive", "backward_only_adaptive_muthu","backward_only_leap", "backward_only_leap_muthu", "backward_only_leap_adaptive_muthu"] #,

print("Available modes : "+",".join(available_variants))
'''
for mode in available_variants:
    print("Running queries for dataset '"+dataset+"' using '"+mode+"' mode.")
    cmd = './build/query-index ../data/'+dataset+' Queries/'+queries+' '+mode+' 0 0 '+number_of_results+' ' + timeout + ' > '+output_folder+'/tmp_'+mode+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv'
    print(cmd)
    os.system(cmd)
'''
#SECOND PART
success=True
for mode_idx, mode in enumerate(available_variants):
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
sigmod21_performance = []
sigmod21_adaptive_performance = []
one_ring_muthu_leap_performance = []
one_ring_muthu_adaptive_leap_performance = []
backward_only_performance = []
backward_only_muthu_performance = []
backward_only_adaptive_performance = []
backward_only_muthu_adaptive_performance = []
backward_only_leap_performance = []
backward_only_leap_muthu_performance = []
backward_only_leap_adaptive_muthu_performance = []

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
    aux11=lists_of_rows[10][index][0].split(";")
    if aux1[1] != aux2[1] or aux1[1] != aux3[1] \
    or aux1[1] != aux4[1] or aux1[1] != aux5[1] \
    or aux1[1] != aux6[1] or aux1[1] != aux7[1] \
    or aux1[1] != aux8[1] or aux1[1] != aux9[1] \
    or aux1[1] != aux10[1] or aux1[1] != aux11[1]:
        num_of_results_error=num_of_results_error+1
    #div by 1000000000 just like in generate_output.cpp
    sigmod21_performance.append(float(aux1[2])/1000000000)
    sigmod21_adaptive_performance.append(float(aux2[2])/1000000000)
    one_ring_muthu_leap_performance.append(float(aux3[2])/1000000000)
    one_ring_muthu_adaptive_leap_performance.append(float(aux4[2])/1000000000)
    backward_only_performance.append(float(aux5[2])/1000000000)
    backward_only_muthu_performance.append(float(aux6[2])/1000000000)
    backward_only_adaptive_performance.append(float(aux7[2])/1000000000)
    backward_only_muthu_adaptive_performance.append(float(aux8[2])/1000000000)
    backward_only_leap_performance.append(float(aux9[2])/1000000000)
    backward_only_leap_muthu_performance.append(float(aux10[2])/1000000000)
    backward_only_leap_adaptive_muthu_performance.append(float(aux11[2])/1000000000)

print("****** Number of different results: ", num_of_results_error)
if num_of_results_error != 0:
    quit()

#THIRD PART. Creating individual measures per query type.
print("****** Creating type specific files to be use later for plotting.")
query_types = ['J3', 'J4', 'P2', 'P3', 'P4', 'S1', 'S2', 'S3', 'S4', 'T2', 'T3', 'T4', 'TI2', 'TI3', 'TI4', 'Tr1', 'Tr2' ]
types_per_queries_file = open('plots/box-plot-sigmod21/query_types.txt', 'r')
types_per_queries = types_per_queries_file.readlines()
for query_type in query_types: #Per each type of query.
    query_type_data = {} #dictionary
    query_type_file = "plots/matplotlib/"+query_type+".txt"
    print("Creating "+ query_type_file + " file.")
    for variant in available_variants: #Per each ring variant.
        query_type_data[variant] = [] #empty list
        print('Processing : ' +output_folder+'/tmp_'+variant+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv')
        #First we add all the competitors to the plotting files.
        #Function that appends existing pre-calculated values per query type.
        #query_type_data['CompactLTJ'] = TODO
        with open(output_folder+'/tmp_'+variant+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv', 'r') as csvfile:
            # creating a csv reader object
            csvreader = csv.reader(csvfile)
            #assert len(types_per_queries) == len(csvreader)
            # extracting each data row one by one
            for i, row in enumerate(csvreader):
                if types_per_queries[i].strip() == query_type+".txt":
                    aux = row[0].split(";")
                    query_type_data[variant].append(float(aux[2])/1000000000)#third argument is the execution time.
    #print(query_type_data)
    with open(query_type_file, "w") as csv_output_file:
        writer = csv.writer(csv_output_file)
        writer.writerow(query_type_data.keys())
        writer.writerows(zip(*query_type_data.values())) # * is the unpack operator.
'''
#FOURTH PART. Producing the input files for the sigmod21 plotting mechanism in overleaf. (TO BE DEPRECATED) TODO: comparare files with above and deprecate if success.
print("****** Generating input files for sigmod21 plotting mechanism (overleaf).")
for mode in available_variants:
    command='./plots/box-plot-sigmod21/generate_output '+output_folder+'/tmp_'+mode+'_'+dataset+'_'+number_of_results+'_'+timeout+'.csv plots/box-plot-sigmod21/query_types.txt'
    print("Running command: "+command)
    os.system(command)

#FOURTH PART (DEPRECATED)
print("****** Plotting variants using matplotlib.")

d = {'Orig': sigmod21_performance,
    'Muthu': one_ring_muthu_leap_performance,
    'Muthu Adapt': one_ring_muthu_adaptive_leap_performance,
    'Backward' : backward_only_performance,
    'B. Muthu' : backward_only_muthu_performance,
    'B. Adapt' : backward_only_adaptive_performance,
    'B. Adapt Muthu' : backward_only_muthu_adaptive_performance,
    #'Sigmod21_adaptive': sigmod21_adaptive_performance,
    'B. Leap': backward_only_leap_performance,
    'B. Leap Muthu' : backward_only_leap_muthu_performance,
    'B. Leap Adapt Muthu' : backward_only_leap_adaptive_muthu_performance
    }

df = pd.DataFrame(data = d)
plt.ylim(0, 0.005)
df.boxplot()

plt.show()
plt.savefig('plot.pdf', format='pdf')
#plot.get_figure().savefig('plot.pdf', format='pdf')
#https://stackoverflow.com/questions/69828508/warning-ignoring-xdg-session-type-wayland-on-gnome-use-qt-qpa-platform-wayland
'''

print("****** Ending automated test.")
