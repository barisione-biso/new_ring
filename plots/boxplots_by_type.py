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

labels_ = ['Emptyheaded', 'Jena-LTJ', 'RDF3X', 'Virtuoso', 'Blazegraph', 'CompactLTJ', 'Qdag BFS', 'RDFCSA', 'MillenniumDB' , 'Ring', 'Ring Adaptive', 'Ring Muthu', 'Ring Adaptive Muthu', 'URing', 'URing Muthu', 'URing Adaptive', 'URing Adaptive Muthu', 'URing (leap)', 'URing (leap) Muthu', 'URing (leap) Adaptive Muthu']

colors = ['lightgreen','brown', 'lightblue', 'black', 'red', 'blue', 'orange', 'purple', 'green', 'violet', 'olive',  'teal', 'cyan', 'lime', 'lightgray', 'darkgray', 'tan', 'pink', 'yellow']
k = 0
i = 0
j = 0
max_rows=3
max_columns=6
#first_bplot is declared here as a list but later replaced to store the 'artist' of the first boxplot.
#The reason to do this is because the last bplot is deleted and therefore it cant be used as a valid handled for the legend.
first_bplot = []
print("****** Plotting Starts.")
#plt.figure(figsize=(12.8,9.6))
plt.rcParams['figure.figsize'] = [12.8,9.6]
fig, axes = plt.subplots(max_rows,max_columns) #2 rows with 4 columns
while k < len(query_types):
    ymax = 0.01
    #hardcoded stuffs >>
    if (i == 0 and j == max_columns - 1): #Omit plot 0,5 (Legend will be here)
        j = 0
        i = i + 1
    if i == 1:
        ymax = 0.005
    elif i == 2:
        ymax = 0.04
    #hardcoded stuffs <<
    
    print('Working with file '+query_type_files[k])
    data = pd.read_csv(query_type_files[k])
    #Deleting URing (leap) variants from plot.
    del data['backward_only_leap_adaptive_muthu']
    del data['backward_only_leap_muthu']
    del data['backward_only_leap']
    competitors_data = pd.read_csv('competitors_measures/'+query_types[k]+'_source_competitors.txt')
    concated_data = pd.concat([competitors_data, data], axis=1,join='inner')
    bplot = axes[i, j].boxplot(
        concated_data,
        patch_artist=True, # fill with color
        showfliers=False
    )
    if not first_bplot:
        first_bplot = bplot
    #Setting the colors.
    for patch, color in zip(bplot['boxes'], colors):
        patch.set_facecolor(color)

    #print ( 'i = ' + str(i) + ' , j = ' + str(j))
    axes[i, j].set_ylim(0, ymax)
    # yticks
    #locs,labels = yticks()
    #yticks(locs, map(lambda x: x, locs))
    axes[i, j].set_title(query_types[k])
    axes[i, j].get_xaxis().set_visible(False)
    if j == 0:
        axes[i, j].set_ylabel("Time (secs)")
    else:
       axes[i, j].set_yticklabels([])
    j = j + 1
    if j >= max_columns:
        j = 0
        i = i + 1
    k = k + 1
    #df.boxplot()

fig.delaxes(axes[0,5]) #remove the last subplot of the first row (unused)
#fig.suptitle("Tradeoff space-time")

handles_ = [first_bplot["boxes"][0],first_bplot["boxes"][1],first_bplot["boxes"][2],first_bplot["boxes"][3],first_bplot["boxes"][4],first_bplot["boxes"][5],first_bplot["boxes"][6],first_bplot["boxes"][7],first_bplot["boxes"][8],first_bplot["boxes"][9],first_bplot["boxes"][10],first_bplot["boxes"][11],first_bplot["boxes"][12],first_bplot["boxes"][13],first_bplot["boxes"][14],first_bplot["boxes"][15],first_bplot["boxes"][16]] #,first_bplot["boxes"][17],first_bplot["boxes"][18], first_bplot["boxes"][19]]
fig.legend(handles = handles_, labels = labels_, loc="upper right", fontsize=7, ncol=1, bbox_to_anchor=(0.99, 1))
fig.tight_layout() #Adds padding among subplots with enough size to show x & y labels.
plt.savefig('boxplots_by_type.svg', format='svg',dpi=500)
plt.show()

print("****** Plotting Ends.")
