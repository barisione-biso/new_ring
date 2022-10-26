#Started as a copy of automated_test.py
import os
import csv

import pandas as pd
import matplotlib.pyplot as plt
from pylab import yticks
import numpy as np

import argparse

# Initialize parser
parser = argparse.ArgumentParser()
 
# Adding optional argument
#parser.add_argument("-f", "--First", help = "First csv in the plot.", required=True)
#parser.add_argument("-o", "--OutputFolder", help = "Output folder.")

# Read arguments from command line
args = parser.parse_args()

query_type_files = ["matplotlib/P2.txt",
"matplotlib/P3.txt",
"matplotlib/P4.txt",
"matplotlib/T2.txt",
"matplotlib/TI2.txt",
"matplotlib/T3.txt",
"matplotlib/TI3.txt",
"matplotlib/J3.txt",
"matplotlib/T4.txt",
"matplotlib/TI4.txt",
"matplotlib/J4.txt",
"matplotlib/Tr1.txt",
"matplotlib/Tr2.txt",
"matplotlib/S1.txt",
"matplotlib/S2.txt",
"matplotlib/S3.txt",
"matplotlib/S4.txt"]

#query_types = ['J3', 'J4', 'P2', 'P3', 'P4', 'S1', 'S2', 'S3', 'S4', 'T2', 'T3', 'T4', 'TI2', 'TI3', 'TI4', 'Tr1', 'Tr2' ]
query_types = ['P2', 'P3', 'P4', 'T2', 'TI2', 'T3', 'TI3', 'J3', 'T4', 'TI4', 'J4', 'Tr1', 'Tr2', 'S1', 'S2', 'S3', 'S4' ]
k = 0
i = 0
j = 0
max_rows=3
max_columns=6
print("****** Plotting Starts.")
fig, axes = plt.subplots(max_rows,max_columns) #2 rows with 4 columns
fig.tight_layout(h_pad=2) #Adds padding among subplots with enough size to show x & y labels.
subplots = []
while k < len(query_types):
    print('Working with file '+query_type_files[k])
    data = pd.read_csv(query_type_files[k])
    competitors_data = pd.read_csv('competitors_measures/'+query_types[k]+'_source_competitors.txt')
    concated_data = pd.concat([competitors_data, data], axis=1,join='inner')
    bplot = axes[i, j].boxplot(
        concated_data,
        patch_artist=True # fill with color
    )
    subplots.append(bplot)
    ymax = 0.01

    #print ( 'i = ' + str(i) + ' , j = ' + str(j))
    #hardcoded ymax >>
    if (j == 5 and i == 0) or (i == 1 and j != 5):
        ymax = 0.005
    elif (j == 5 and i == 1) or i == 2:
        ymax = 0.04
    #hardcoded ymax <<
    axes[i, j].set_ylim(0, ymax)
    # yticks
    #locs,labels = yticks()
    #yticks(locs, map(lambda x: x, locs))
    axes[i, j].set_title(query_types[k])
    axes[i, j].get_xaxis().set_visible(False)
    axes[i, j].set_ylabel("Time (secs)")
    j = j + 1
    if j >= max_columns:
        j = 0
        i = i + 1
    k = k + 1
    #df.boxplot()

fig.delaxes(axes[2,5]) #remove the last subplot as the 18th one is unused.
#fig.suptitle("Tradeoff space-time")

#Setting the colors.
colors = ['brown', 'lightblue', 'black', 'red', 'blue', 'orange', 'purple', 'green', 'violet', 'olive',  'teal', 'cyan', 'lime', 'lightgray', 'darkgray', 'tan', 'pink', 'yellow']
for subplot in subplots:
        for patch, color in zip(subplot['boxes'], colors):
            patch.set_facecolor(color)

fig.legend(labels = ['Emptyheaded', 'Jena-LTJ', 'RDF3X', 'Virtuoso', 'Blazegraph', 'CompactLTJ', 'Qdag', 'PODS', 'PODS Adaptive', 'PODS Muthu', 'PODS Muthu adaptive', 'Backward', 'Backward Muthu', 'Backward Adaptive', 'Backward Adaptive Muthu', 'Backward (leap)', 'Backward (leap) Muthu', 'Backward (leap) adaptive Muthu'], loc="lower right", fontsize=8)
# Finally, add a basic legend
N = 500
fig.text(0.80, 0.08, f'{N} Random Numbers',
         backgroundcolor=colors[0], color='black', weight='roman',
         size='x-small')
fig.text(0.80, 0.045, 'IID Bootstrap Resample',
         backgroundcolor=colors[1],
         color='white', weight='roman', size='x-small')
fig.text(0.80, 0.015, '*', color='white', backgroundcolor='silver',
         weight='roman', size='medium')
fig.text(0.815, 0.013, ' Average Value', color='black', weight='roman',
         size='x-small')
plt.show()

print("****** Plotting Ends.")
