import numpy as np
import matplotlib.pyplot as plt
from pylab import yticks
#'Orig'
bytes_space_usage_per_structure = np.asarray([11.6])
median_per_structure = np.asarray([2989500])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Sigmod Adaptive'
bytes_space_usage_per_structure = np.asarray([11.6])
median_per_structure = np.asarray([2948000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Muthu'
bytes_space_usage_per_structure = np.asarray([26.7])
median_per_structure = np.asarray([2662000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Muthu adaptive'
bytes_space_usage_per_structure = np.asarray([26.7])
median_per_structure = np.asarray([2540000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward'
bytes_space_usage_per_structure = np.asarray([23.2])
median_per_structure = np.asarray([1796000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward Muthu'
bytes_space_usage_per_structure = np.asarray([38.3])
median_per_structure = np.asarray([1492000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward Adaptive'
bytes_space_usage_per_structure = np.asarray([23.2])
median_per_structure = np.asarray([1739500])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward Adaptive Muthu'
bytes_space_usage_per_structure = np.asarray([38.3])
median_per_structure = np.asarray([1538500])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward (leap)'
bytes_space_usage_per_structure = np.asarray([23.2])
median_per_structure = np.asarray([2871500])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward (leap) Muthu'
bytes_space_usage_per_structure = np.asarray([38.3])
median_per_structure = np.asarray([2553000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Backward (leap) adaptive Muthu'
bytes_space_usage_per_structure = np.asarray([38.3])
median_per_structure = np.asarray([2805000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

#'Compact LTJ'
bytes_space_usage_per_structure = np.asarray([45.6])
median_per_structure = np.asarray([459000])

plt.scatter(
    x=bytes_space_usage_per_structure,
    y=median_per_structure
)

# ytikcs
locs,labels = yticks()
yticks(locs, map(lambda x: x, locs))

plt.title("Tradeoff space-time")
plt.legend(['TODS', 'TODS Adaptive', 'TODS Muthu', 'TODS Muthu adaptive', 'Backward', 'Backward Muthu', 'Backward Adaptive', 'Backward Adaptive Muthu', 'Backward (leap)', 'Backward (leap) Muthu', 'Backward (leap) adaptive Muthu', 'Compact LTJ'])
plt.xlabel("Bytes per triple")
plt.ylabel("Median (ms)")
#plt.text(
#    3.2,
#    55,
#    "Size of marker = profit margin\n" "Color of marker = sugar content",
#)

plt.show()
