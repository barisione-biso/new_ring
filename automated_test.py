import os
import csv

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

#FIRST PART.
sigmod_rows = []
one_ring_muthu_leap_rows = []
#Wikidata filtered enumerated automated test.
sigmod_filename="tmp_queries_wikidata_enumerated_sigmod21.csv"
one_ring_muthu_leap_filename="tmp_queries_wikidata_enumerated_one_ring_muthu_leap.csv"
'''print("Running queries for wikidata-filtered-enumerated.dat.ring with 'sigmod21' mode.")
cmd = './build/query-index ../data/wikidata-filtered-enumerated.dat.ring Queries/Queries-wikidata-benchmark.txt sigmod21 0 0 > '+sigmod_filename
os.system(cmd)
print("Running queries for wikidata-filtered-enumerated.dat.ring with 'one_ring_muthu_leap' mode.")
cmd = './build/query-index ../data/wikidata-filtered-enumerated.dat.ring Queries/Queries-wikidata-benchmark.txt one_ring_muthu_leap 0 0 > '+one_ring_muthu_leap_filename
os.system(cmd)
'''

#SECOND PART

#Opening sigmod file.
with open(sigmod_filename, 'r') as csvfile:
    # creating a csv reader object
    csvreader = csv.reader(csvfile)

    # extracting each data row one by one
    for row in csvreader:
        sigmod_rows.append(row)

    # get total number of rows
    print("Total # of sigmod_rows: %d"%(csvreader.line_num))

#Opening one_ring_muthu_leap file.
with open(one_ring_muthu_leap_filename, 'r') as csvfile:
    # creating a csv reader object
    csvreader = csv.reader(csvfile)

    # extracting each data row one by one
    for row in csvreader:
        one_ring_muthu_leap_rows.append(row)

    # get total number of rows
    print("Total # of one_ring_muthu_leap_rows: %d"%(csvreader.line_num))

print("Checking for result correctness.")

num_of_results_error = 0
sigmod_performance = []
one_ring_muthu_leap_performance = []
for index, sigmod_row in enumerate(sigmod_rows):
    # working with the second column of the row
    aux=sigmod_row[0].split(";")
    aux2=one_ring_muthu_leap_rows[index][0].split(";")
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
