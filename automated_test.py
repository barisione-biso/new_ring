import os
import csv

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

#FIRST PART.
rows = [[]] #A list of lists.
#Wikidata filtered enumerated automated test.
dataset="wikidata-filtered-enumerated.dat.ring"
queries="Queries-wikidata-benchmark.txt"
available_modes = ["sigmod21", "one_ring_muthu_leap"]

print("Available modes : "+",".join(available_modes))
for mode in available_modes:
    print("Running queries for dataset '"+dataset+"' using '"+mode+"' mode.")
    cmd = './build/query-index ../data/'+dataset+' Queries/'+queries+' '+mode+' 0 0 > '+mode
    os.system(cmd)

#SECOND PART

for mode_idx, mode in enumerate(available_modes):
    #Opening sigmod file.
    with open("tmp_"+mode+".csv", 'r') as csvfile:
        # creating a csv reader object
        csvreader = csv.reader(csvfile)

        # extracting each data row one by one
        for row in csvreader:
            rows[mode_idx].append(row)

        # get total number of rows
        print("Total # of "+mode+" rows: %d"%(csvreader.line_num))

print("Checking for result correctness.")

#TODO: FROM HERE ONWARDS CODE GENERALIZATION IS NEEDED.
#rows[0] : SIGMOD21, rows[1]: muthu
num_of_results_error = 0
sigmod_performance = []
one_ring_muthu_leap_performance = []
for index, sigmod_row in enumerate(rows[0]):
    # working with the second column of the row
    aux=sigmod_row[0].split(";")
    aux2=rows[1][index][0].split(";")
    if aux[1] != aux2[1]:
        num_of_results_error=num_of_results_error+1

    sigmod_performance.append(int(aux[2]))
    one_ring_muthu_leap_performance.append(int(aux2[2]))

print("Number of different results: ", num_of_results_error)

#THIRD PART
print("Plotting the variants.")

d = {'Sigmod21': sigmod_performance, 'One Ring + Muthu + Leap': one_ring_muthu_leap_performance}
#print(d)
df = pd.DataFrame(data = d)
plot = df.boxplot(column=['Sigmod21', 'One Ring + Muthu + Leap'])
plot.get_figure().savefig('global_plot.pdf', format='pdf')

plot.set_ylim(0.0, 0.5)
plot.get_figure().savefig('y_limit_plot.pdf', format='pdf')
#https://stackoverflow.com/questions/69828508/warning-ignoring-xdg-session-type-wayland-on-gnome-use-qt-qpa-platform-wayland

#deleting tmp files.
for mode in available_modes:
    os.system("rm tmp_"+mode+".csv")