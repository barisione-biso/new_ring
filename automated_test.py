import os
import csv

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

#FIRST PART.
lists_of_rows = [] #A list of lists.
#Wikidata filtered enumerated automated test.
dataset="wikidata-filtered-enumerated.dat.ring" #wikidata.nt.enumerated.ring
queries="Queries-wikidata-benchmark.txt"
available_modes = ["sigmod21", "one_ring_muthu_leap", "one_ring_muthu_leap_adaptive"]
'''
print("Available modes : "+",".join(available_modes))
for mode in available_modes:
    print("Running queries for dataset '"+dataset+"' using '"+mode+"' mode.")
    cmd = './build/query-index ../data/'+dataset+' Queries/'+queries+' '+mode+' 0 0 > tmp_'+mode+'.csv'
    print(cmd)
    os.system(cmd)
'''
#SECOND PART
success=True
for mode_idx, mode in enumerate(available_modes):
    #Opening sigmod file.
    with open("tmp_"+mode+".csv", 'r') as csvfile:
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

print("Checking for result correctness.")
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

print("Number of different results: ", num_of_results_error)

#THIRD PART
print("Plotting the variants.")

d = {'Sigmod21': sigmod_performance, 
    'One Ring + Muthu + Leap': one_ring_muthu_leap_performance,
    'One Ring + Muthu + Leap adaptative': one_ring_muthu_adaptative_leap_performance}
#print(d)
df = pd.DataFrame(data = d)
plot = df.boxplot(column=['Sigmod21', 'One Ring + Muthu + Leap', 'One Ring + Muthu + Leap adaptative'], grid=True, return_type='axes')
#plot.get_figure().savefig('global_plot.pdf', format='pdf')

#plot.set_ylim(0.0, 100.0)
plot.get_figure().savefig('y_limit_plot.pdf', format='pdf')
#https://stackoverflow.com/questions/69828508/warning-ignoring-xdg-session-type-wayland-on-gnome-use-qt-qpa-platform-wayland

#deleting tmp files.
#for mode in available_modes:
#    os.system("rm tmp_"+mode+".csv")