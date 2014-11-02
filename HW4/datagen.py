#!/usr/bin/python3
import matplotlib.pyplot as plt
from StringIO import StringIO
import csv
data = []
with open('kernel.log', 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in spamreader:
        print ', '.join(row)
kerneldata = np.genfromtxt(StringIO(data), delimiter=" ")
x = [1, 2, 3, 4]
y = [1, 4, 9, 6]
labels = ['Starttime', 'Endtime']

plt.plot(x, y, 'ro')
# You can specify a rotation for the tick labels in degrees or with keywords.
plt.xticks(x, labels, rotation='vertical')
# Pad margins so that markers don't get clipped by the axes
plt.margins(0.2)
# Tweak spacing to prevent clipping of tick-labels
plt.subplots_adjust(bottom=0.15)
plt.show()
