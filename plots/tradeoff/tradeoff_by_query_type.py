from pandas import *
import numpy as np
import matplotlib.pyplot as plt
from pylab import yticks

#Markers: ['o', '.', ',', 'x', '+', 'v', '^', '<', '>', 's', 'd']
bytes_space_usage_per_structure = np.asarray([11.6, 11.6, 26.7, 26.7, 23.2, 38.3, 23.2, 38.3, 23.2, 38.3, 38.3, 45.6])

data = read_csv('median_per_query_type.csv')

k = 0
i = 0
j = 0
max_rows=3
max_columns=6
query_types = ['J3', 'J4', 'P2', 'P3', 'P4', 'S1', 'S2', 'S3', 'S4', 'T2', 'T3', 'T4', 'Ti2', 'Ti3', 'Ti4', 'Tr1', 'Tr2' ]
fig, axes = plt.subplots(max_rows,max_columns) #2 rows with 4 columns
fig.tight_layout(h_pad=2) #Adds padding among subplots with enough size to show x & y labels.
while k < 17:
    for structure_index, median in enumerate(data[query_types[k]]):
        if structure_index in [0, 4, 11]:
            marker_value = 'd'
        elif structure_index in [3, 7]:
            marker_value = 'x'
        else:
            marker_value = 'o'
        axes[i, j].scatter(
            x=bytes_space_usage_per_structure[structure_index],
            y=median,
            marker=marker_value
        )
        # ytikcs
        locs,labels = yticks()
        yticks(locs, map(lambda x: x, locs))
        axes[i, j].set_title(query_types[k])
        axes[i, j].set_xlabel("Bytes per triple")
        axes[i, j].set_ylabel("Median (ms)")
    j = j + 1
    if j >= max_columns:
        j = 0
        i = i + 1
    k = k + 1

fig.delaxes(axes[2,5]) #remove the last subplot as the 18th one is unused.
#fig.suptitle("Tradeoff space-time")
fig.legend(['PODS', 'PODS Adaptive', 'PODS Muthu', 'PODS Muthu adaptive', 'Backward', 'Backward Muthu', 'Backward Adaptive', 'Backward Adaptive Muthu', 'Backward (leap)', 'Backward (leap) Muthu', 'Backward (leap) adaptive Muthu', 'Compact LTJ'], loc="lower right")
plt.show()