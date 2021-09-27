"""
enotools.py

Collection of python methods for basic analysis with enophone with data
format compatible with BrainFlow.

:copyright: © 2021 Mindset Innovation, inc.  All rights reserved
:author: David Doyon
"""

import numpy as np
import sys
from scipy.signal import butter, filtfilt

def filter(signal, lowerbound, upperbound, nyquist_f, filtertype, order):
    bandpass_filter_window = np.array([lowerbound, upperbound]) / nyquist_f

    a, b = butter(N=order, Wn=bandpass_filter_window, btype=filtertype)
    filtered = filtfilt(a, b, signal)
    return filtered

def signal_filtering(data,filter_cut=None,bandpass_range=None,bandstop_range=None, samplerate=250):

    local_data = data.copy()

    if bandpass_range:
        if bandpass_range[0] <= 0:
            print('ERROR: First value of the bandpass filter must be above 0. The value is: {}'.format(bandpass_range[0]))
            sys.exit(1)
        if bandpass_range[1] >= 125:
            print('ERROR: Second value of the bandpass filter must be below 125. The value is: {}'.format(bandpass_range[1]))
            sys.exit(1)
        try:
            for i in (1,2,3,4):
                local_data[i] = filter(local_data[i], bandpass_range[0], bandpass_range[1], samplerate/2, filtertype='bandpass', order=4)
        except BaseException:
            print('ERROR: Bandpass filtering failed. Try setting different parameters.')
            sys.exit(1)

        if filter_cut and not(bandstop_range):
            if filter_cut*3 >= local_data.shape[1]:
                print('ERROR: filter_cut too large. It should be less than one third of the data length.')
                sys.exit(1)
            local_data = local_data[:,filter_cut:-filter_cut]

    if bandstop_range:
        if bandstop_range[0] <= 0:
            print('ERROR: First value of the bandstop filter must be above 0. The value is: {}'.format(bandstop_range[0]))
            sys.exit(1)
        if bandstop_range[1] >= 125:
            print('ERROR: Second value of the bandstop filter must be below 125. The value is: {}'.format(bandstop_range[1]))
            sys.exit(1)

        try:
            for i in (1,2,3,4):
                local_data[i] = filter(local_data[i], bandstop_range[0], bandstop_range[1], samplerate/2, filtertype='bandstop', order=4)
        except BaseException:
            print('ERROR: Bandpass filtering failed. Try setting different parameters.')
            sys.exit(1)

        if filter_cut:
            if filter_cut*3 >= local_data.shape[1]:
                print('ERROR: filter_cut too large. It should be less than one third of the data length.')
                sys.exit(1)
            local_data = local_data[:,filter_cut:-filter_cut]

    return local_data

def detect_mains(data):

    fft, freqs = calc_fft(data)

    ix_60 = np.logical_and(freqs >= 59.8, freqs < 60.2)
    ix_50 = np.logical_and(freqs >= 49.8, freqs < 50.2)

    for i in range(0,fft.shape[1]):
        chan_fft = fft[:,i]
        xref_50 = np.sum(chan_fft[ix_50])
        xref_60 = np.sum(chan_fft[ix_60])

    power_50 = np.mean(np.mean(xref_50))
    power_60 = np.mean(np.mean(xref_60))

    if power_50 > power_60:
        print('50Hz Environment Detected.')
        return [49,51]
    else:
        print('60Hz Environment Detected.')
        return [59,61]

def quality(data):
    #Should only be used on raw, unreferenced signal.
    fft, freqs = calc_fft(data)

    quality = np.zeros(fft.shape[1])
    power = np.zeros(fft.shape[1])

    ix_power = np.logical_and(freqs >= 10, freqs < 40)

    for i in range(0,fft.shape[1]):
        chan_fft = fft[:,i].copy()
        power[i] = np.sum(chan_fft[ix_power])

    #Using A1 or A2 as reference for "best contact"
    #The quality metric will only be relevant as long as one of A1 or A2 touches
    ref = np.max(power[0:2])

    for i in range(0,fft.shape[1]):
        quality[i] = 1-abs(1-power[i]/ref)

    if quality[0] > quality[1]:
        quality[0] = np.max(quality[1:])
    else:
        quality[1] = np.max(quality[(0,2,3),None])

    return np.round(quality*100,1)

def referencing(data, mode):

    local_data = data.copy()

    if mode=='mastoid':
        reference = (local_data[1,:]+local_data[2,:])/2
    elif mode=='Left Cushion' or mode=='A1':
        reference = local_data[2,:].copy()
    elif mode=='Right Cushion' or mode=='A2':
        reference = local_data[1,:].copy()
    elif mode=='Top Right' or mode=='C4':
        reference = local_data[3,:].copy()
    elif mode=='Top Left' or mode=='C3':
        reference = local_data[4,:].copy()
    elif mode=='Average':
        reference = np.mean(local_data[(1,2,3,4),:],axis=0).copy()

    local_data[(1,2,3,4),:] -= reference

    return local_data

def calc_fft(data):

    local_data = data[(1,2,3,4),:].copy()

    ### FFT ###
    samplerate = 250
    nsamples = local_data.shape[1]
    win = {'none' : 1,
           'hanning' : np.hanning(nsamples),
           'hamming' : np.hamming(nsamples)}['hanning']
    win = np.array([win]).T

    fft = np.abs(np.fft.rfft(local_data.T*win, axis=0, norm=None)) * 2 / nsamples
    freqs = np.fft.rfftfreq(nsamples, 1./samplerate)

    return fft, freqs
