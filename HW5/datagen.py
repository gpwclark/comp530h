#!/usr/bin/python
import matplotlib.pyplot as plt
import numpy as np
import csv
import sys
data = []
with open(sys.argv[1], 'r') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in spamreader:
        if 'vmlogger:' in row and 'MM' in row:
            parsedrow = []
            for item in row[4:]:
                try:
                    parsedrow.append(int(item))
                except ValueError:
                    pass
            data.append(np.array(parsedrow))
#print('Example input string that we are parsing:')
#print('Nov 24 21:44:24 localhost kernel: vmlogger: MM c17a3e40 PAGE 738767 PAGE_OFFSET 0 PFN 494180 TIME 814')
#MM PAGE PAGE_OFFSET PFN TIME

data = np.array(data)
#plot the data
#time_ns = data[][4]
#print(data[1][4] - time_ns)
x = []
y = []
xlabels = ['Page Offset']
#print(data[0])
for line in data:
    x.append(line[2]) # the time is added to the x
    y.append(line[-1])

#print(x[0])
#print(y[0])
x = np.array(x)
y = np.array(y)
plt.plot(x, y, 'ro')
# You can specify a rotation for the tick labels in degrees or with keywords.
#plt.xticks(x, xlabels, rotation='vertical')

plt.title('Execution of User Round Robin Processes over time')
plt.ylabel('Fault Time in NS')
plt.xlabel('Page Offset')
# Pad margins so that markers don't get clipped by the axes
plt.margins(0.2)
# Tweak spacing to prevent clipping of tick-labels
plt.subplots_adjust(bottom=0.15)
plt.show()

