# -*- coding: utf-8 -*-
"""
Created on Mon Nov  2 11:08:10 2015

@author: jyoung
"""

import matplotlib.pyplot as plt
import numpy as np
import scipy.stats
import SimpleITK as sitk


class IndexTracker(object):
    def __init__(self, ax, X, Xover=None):
        self.ax = ax
        self.X = X
        self.Xover = Xover
        self.slices, numrows, numcols = X.shape
        self.ind = self.slices//2
        
        def format_coord(x, y):
            col = int(x + 0.5)
            row = int(y + 0.5)
            if col>=0 and col<numcols and row>=0 and row<numrows:
                z = X[self.ind, row, col]
                return 'x=%1.2f, y=%1.2f, z=%1.2f' %(x, y, z)
            else:
                return 'x=%1.2f, y=%1.2f' %(x, y)
        
        if self.Xover == None:
            ax.format_coord = format_coord
            self.im = ax.imshow(self.X[self.ind,:,:], cmap=plt.cm.Greys_r)
        else:
            self.im = ax.imshow(self.X[self.ind,:,:], cmap=plt.cm.Greys_r)
            self.imOver = ax.imshow(self.Xover[self.ind,:,:], alpha=0.5)
            
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
        if self.Xover != None:
            self.imOver.set_data(self.Xover[self.ind,:,:])
            self.imOver.axes.figure.canvas.draw()


def show_imgs(imgstack, imgstack2=None):
    """Uses imgscroll to show stack of images in a new window with 
    mouse scrolling. If a 2nd image stack is provided, then it will 
    be overlaid on top of the 1st."""
    X = sitk.GetArrayFromImage(imgstack)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    if imgstack2 != None:
        X2 = sitk.GetArrayFromImage(imgstack2)
        maskVal = scipy.stats.mode(X2.flatten())[0][0]
        Xmask = np.ma.masked_where(X2 == maskVal, X2)
        tracker = IndexTracker(ax, X, Xmask)
    else:
        tracker = IndexTracker(ax, X)
    fig.canvas.mpl_connect('scroll_event', tracker.onscroll)
    plt.show()

