# -*- coding: utf-8 -*-
"""
Created on Mon Nov  2 11:08:10 2015

@author: jyoung
"""

import matplotlib.pyplot as plt
import numpy as np


class IndexTracker(object):
    def __init__(self, ax, X):
        self.ax = ax
        self.X = X
        self.slices, numrows, numcols = X.shape
        self.ind = 0
        
        def format_coord(x, y):
            col = int(x + 0.5)
            row = int(y + 0.5)
            if col>=0 and col<numcols and row>=0 and row<numrows:
                z = X[self.ind, row, col]
                return 'x=%1.2f, y=%1.2f, z=%1.2f' %(x, y, z)
            else:
                return 'x=%1.2f, y=%1.2f' %(x, y)
        ax.format_coord = format_coord
        
        self.im = ax.imshow(self.X[self.ind, :, :], cmap=plt.cm.Greys_r)
        self.update()

    def onscroll(self, event):
        if event.button == 'up':
            self.ind = np.clip(self.ind+1, 0, self.slices-1)
        else:
            self.ind = np.clip(self.ind-1, 0, self.slices-1)

        self.update()

    def update(self):
        self.im.set_data(self.X[self.ind, :, :])
        self.ax.set_title('slice %s' %self.ind)
        self.im.axes.figure.canvas.draw()
