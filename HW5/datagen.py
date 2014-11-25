#!/usr/bin/python
import matplotlib.pyplot as plt
import numpy as np
import csv
import sys
def make_plot(data, index):
    data = np.array(data)
    x = []
    y = []
    for line in data:
        x.append(line[index]) # the time is added to the x
        y.append(line[-1])

    x = np.array(x)
    y = np.array(y)
    fig = plt.figure()
    ax = fig.add_subplot(2,1,1)
    fig.suptitle('Page faults and the respective handling time ')
    fig.add_axes(ylabel='Fault Time in NS')
    fig.add_axes(xlabel='Page Offset')
    fig.subplots_adjust(bottom=0.15)
    line, = ax.plot(x,y, 'ro')
    ax.set_yscale('log')
    plt.show()



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
print(data[0])
make_plot(data, 0) # this is x logical page
make_plot(data, 1) # this is x page_offset
make_plot(data, 2) # this is x PFN
