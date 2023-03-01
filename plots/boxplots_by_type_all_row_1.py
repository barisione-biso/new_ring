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

query_type_files = [
"matplotlib/P2.txt",
"matplotlib/P3.txt",
"matplotlib/P4.txt",
"matplotlib/T2.txt",
"matplotlib/TI2.txt"]


query_types = ['P2', 'P3', 'P4', 'T2', 'TI2']
labels_ = ['URing Adaptive Muthu', 'URing Adaptive', 'CompactLTJ', 'RDFCSA', 'Qdag BFS', 'MillenniumDB', 'Emptyheaded', 'Jena-LTJ', 'RDF3X', 'Virtuoso', 'Blazegraph']
colors = ['purple', 'orange', 'yellow', 'green', 'lightblue', 'brown', 'cyan', 'darkgray', 'tan', 'olive', 'lime']
k = 0
max_rows=1
max_columns=5
#first_bplot is declared here as a list but later replaced to store the 'artist' of the first boxplot.
#The reason to do this is because the last bplot is deleted and therefore it cant be used as a valid handled for the legend.
first_bplot = []
print("****** Plotting Starts.")
plt.rcParams['figure.figsize'] = [18, 2.0125] #[12.8,9.6]
fig, axes = plt.subplots(max_rows,max_columns) #2 rows with 4 columns
while k < len(query_types):
    ymax = 0.05#0.01

    print('Working with file '+query_type_files[k])
    data = pd.read_csv(query_type_files[k])
    #It is important to reorder the dataframe since the CSV containing the measurements is created with a map, therefore the order is not guaranted.
    #Winners should be first (and have the same order than the labels!), because the rest will be deleted.
    new_order = ['backward_only_adaptive_muthu', 'backward_only_adaptive', 'sigmod21', 'sigmod21_adaptive', 'one_ring_muthu_leap', 'one_ring_muthu_leap_adaptive', 'backward_only','backward_only_muthu']
    data = data.reindex(columns=new_order)
    data = data.div(1000000000, level=1, fill_value=0) #changed to seconds.
    #Ring variants with better performance must remain ('URing Adaptive Muthu', 'URing Adaptive'), the rest have to be removed.
    del data['sigmod21']
    del data['sigmod21_adaptive']
    del data['one_ring_muthu_leap']
    del data['one_ring_muthu_leap_adaptive']
    del data['backward_only']
    del data['backward_only_muthu']
    competitors_data = pd.read_csv('competitors_measures/'+query_types[k]+'_source_competitors.txt')
    competitors_new_order = ['CompactLTJ', 'RDFCSA', 'qdag', 'MillDB', 'emptyheaded', 'Jena-LTJ','RDF3X','virtuoso','blazegraph']
    competitors_data = competitors_data.reindex(columns=competitors_new_order)
    concatenated_data = pd.concat([data, competitors_data], axis=1,join='inner')
    bplot = axes[k].boxplot(
        concatenated_data,
        patch_artist=True, # fill with color
        showfliers=False
    )
    if not first_bplot:
        first_bplot = bplot
    #Setting the colors.
    for patch, color in zip(bplot['boxes'], colors):
        patch.set_facecolor(color)

    #print ( 'i = ' + str(i) + ' , j = ' + str(j))
    axes[k].set_ylim(0, ymax)
    # yticks
    #locs,labels = yticks()
    #yticks(locs, map(lambda x: x, locs))
    axes[k].set_title(query_types[k])
    axes[k].get_xaxis().set_visible(False)
    if k == 0:
        axes[k].set_ylabel("Time (secs)")
    else:
        axes[k].set_yticklabels([])
    k = k + 1

handles_ = [first_bplot["boxes"][0],first_bplot["boxes"][1],first_bplot["boxes"][2],first_bplot["boxes"][3],first_bplot["boxes"][4],first_bplot["boxes"][5],first_bplot["boxes"][6],first_bplot["boxes"][7],first_bplot["boxes"][8],first_bplot["boxes"][9],first_bplot["boxes"][10]]
fig.legend(handles = handles_, labels = labels_, loc="upper right", fontsize=8, ncol=1)
#fig.tight_layout() #Adds padding among subplots with enough size to show x & y labels. Commented cause it breaks the legend!
#Removing left padding.
fig.subplots_adjust(
    left=0.042,
    hspace=0.2
)
plt.savefig('boxplots_by_type_all_row_1.pdf', format='pdf',dpi=500)
plt.show()

print("****** Plotting Ends.")
