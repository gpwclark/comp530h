#!/usr/bin/python
import matplotlib.pyplot as plt
import numpy as np
import csv
data = []
with open('kernel.log', 'r') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in spamreader:
        if 'urrsched:' in row and 'PID' in row :
            parsedrow = []
            for item in row[8:]:
                try:
                    parsedrow.append(int(item))
                except ValueError:
                    pass
            data.append(np.array(parsedrow))

#make data into numpy array
data = np.array(data)
print('Example input string that we are parsing:')
print('urr_task_tick PID 3482 with weight 1 timeslice 10 RUNtime 162985252280 ACTUALtime 309785010174 tick_count 16751')
#PID WEIGHT TIMESLICE RUNTIME ACTUALTIME TICK
print(data[:])

#plot the data
starttime_ns = data[0][4]
x = []
y = []
xlabels = ['Time']

for line in data:
    x.append( (line[4] - starttime_ns) * (10**-6) ) # the time is added to the x
    y.append(line[0])

print(x)
print(y)
x = np.array(x)
y = np.array(y)
plt.plot(x, y, 'ro')
# You can specify a rotation for the tick labels in degrees or with keywords.
#plt.xticks(x, xlabels, rotation='vertical')
# Pad margins so that markers don't get clipped by the axes
plt.margins(0.2)
# Tweak spacing to prevent clipping of tick-labels
plt.subplots_adjust(bottom=0.15)
plt.show()
