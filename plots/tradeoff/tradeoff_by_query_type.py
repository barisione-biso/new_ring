from pandas import *
import numpy as np
import matplotlib.pyplot as plt
from pylab import yticks
#import matplotlib.patches as mpatches
import matplotlib.lines as mlines
#Markers: ['o', '.', ',', 'x', '+', 'v', '^', '<', '>', 's', 'd']
bytes_space_usage_per_structure = np.asarray([11.16, 11.16, 26.26, 26.26, 22.32, 37.42, 22.32, 37.42, 15.04, 45.6, 4.9])
#Muthu = +15.1 ? how?
data = read_csv('median_per_query_type.csv')
#plt.rcParams['figure.figsize'] = [12.8,9.6]#[11.2, 8.05]
k = 0
i = 0
j = 0
max_rows=3
max_columns=6
#query_types = ['J3', 'J4', 'P2', 'P3', 'P4', 'S1', 'S2', 'S3', 'S4', 'T2', 'T3', 'T4', 'Ti2', 'Ti3', 'Ti4', 'Tr1', 'Tr2' ]
query_types = ['P2', 'P3', 'P4', 'T2', 'Ti2', 'T3', 'Ti3', 'J3', 'T4', 'Ti4', 'J4', 'Tr1', 'Tr2', 'S1', 'S2', 'S3', 'S4' ]
colors = ['brown', 'lightblue', 'black', 'red', 'blue', 'orange', 'purple', 'green', 'violet', 'olive',  'teal', 'cyan', 'lime', 'lightgray', 'darkgray', 'tan', 'pink', 'yellow']

#TODO: Estos son los mismos colores de los boxplots_by_type pero se ven 'mal'. Falta estandarizar. colors = ['green', 'violet', 'olive',  'teal', 'cyan', 'lime', 'lightgray', 'darkgray', 'tan', 'pink', 'yellow', 'orange', 'purple']
fig, axes = plt.subplots(max_rows,max_columns) #2 rows with 4 columns
fig.tight_layout(h_pad=2) #Adds padding among subplots with enough size to show x & y labels.
#Removing left padding.

fig.subplots_adjust(
    left=0.042,
    hspace=0.4,
    wspace=0.3
)
while k < 17:
    #I created 1 scatter per each struture and add it to a specific Query type. In the future I can try to create 1 scatter plot per all structures (pass an array x, y) and then add it to a specific query type.
    for structure_index, median in enumerate(data[query_types[k]]):
        #Hacks >>
        if structure_index in [0, 4, 9]:
            marker_value = 'd'
        elif structure_index in [3, 7]:
            marker_value = 'x'
        elif structure_index in [10]:
            marker_value = '<'
        else:
            marker_value = 'o'
        #Removing qdag from J3, J4, P3, P4, S2, T3, T4, TI3 and TI4
        if query_types[k] in ['J3', 'J4', 'P3', 'P4', 'S2', 'T3', 'T4', 'Ti2', 'Ti3', 'Ti4']  and structure_index  == 10:
            continue
        #Hacks <<
        axes[i, j].scatter(
            x=bytes_space_usage_per_structure[structure_index],
            y=median,
            marker=marker_value,
            color=colors[structure_index]
            #label=query_types[k]
        )
        # yticks
        locs,labels = yticks()
        yticks(locs, map(lambda x: x, locs))
        axes[i, j].set_title(query_types[k])
        axes[i, j].set_xlabel("Bytes per triple")
        if j == 0:
            axes[i, j].set_ylabel("Median (ns)")
        else:
            axes[i, j].set_ylabel("")
    j = j + 1
    if j >= max_columns:
        j = 0
        i = i + 1
    k = k + 1

fig.delaxes(axes[2,5]) #remove the last subplot as the 18th one is unused.

#taken from : https://stackoverflow.com/questions/47391702/how-to-make-a-colored-markers-legend-from-scratch
handles_ = [mlines.Line2D([], [], color=colors[0], label='Ring',  linestyle='None', marker='d'),
            mlines.Line2D([], [], color=colors[1], label='Ring Adaptive',  linestyle='None', marker='o'),
            mlines.Line2D([], [], color=colors[2], label='Ring Muthu',  linestyle='None', marker='o'),
            mlines.Line2D([], [], color=colors[3], label='Ring Adaptive Muthu',  linestyle='None', marker='x'),
            mlines.Line2D([], [], color=colors[4], label='URing',  linestyle='None', marker='d'),
            mlines.Line2D([], [], color=colors[5], label='URing Muthu',  linestyle='None', marker='o'),
            mlines.Line2D([], [], color=colors[6], label='URing Adaptive',  linestyle='None', marker='o'),
            mlines.Line2D([], [], color=colors[7], label='URing Adaptive Muthu',  linestyle='None', marker='x'),
            #mlines.Line2D([], [], color=colors[8], label='URing (leap)',  linestyle='None', marker='o'),
            #mlines.Line2D([], [], color=colors[9], label='URing (leap) Muthu',  linestyle='None', marker='o'),
            #mlines.Line2D([], [], color=colors[10], label='URing (leap) Adaptive Muthu',  linestyle='None', marker='o'),
            mlines.Line2D([], [], color=colors[8], label='RDFCSA',  linestyle='None', marker='o'),
            mlines.Line2D([], [], color=colors[9], label='Compact LTJ',  linestyle='None', marker='d'),
            mlines.Line2D([], [], color=colors[10], label='Qdag BFS',  linestyle='None', marker='<')
            ]
fig.legend(handles = handles_, loc="lower right", bbox_to_anchor=(0.95, 0.06))
plt.savefig('tradeoff_bytes_per_triple_median.pdf', format='pdf')
plt.show()
