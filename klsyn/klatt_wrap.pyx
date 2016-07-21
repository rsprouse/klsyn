# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 15:01:24 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""

import re
import scipy.io.wavfile
import numpy as np
cimport numpy as np
cimport klsyn.klatt_defs as klatt_defs

# These map param names to index locations in defval. They are defined in the
# symb1 and symb2 arrays in the C code. This is much easier to read.
params_map = {
    'sr': 0,  # C
    'nf': 1,  # C
    'du': 2,  # C
    'ss': 3,  # C
    'ui': 4,  # C
    'rs': 5,  # C
    'f0': 6,
    'av': 7,
    'F1': 8,
    'b1': 9,
    'F2': 10,
    'b2': 11,
    'F3': 12,
    'b3': 13,
    'F4': 14,
    'b4': 15,
    'F5': 16,
    'b5': 17,
    'f6': 18,
    'b6': 19,
    'fz': 20,
    'bz': 21,
    'fp': 22,
    'bp': 23,
    'ah': 24,
    'oq': 25,
    'at': 26,
    'tl': 27,
    'af': 28,
    'sk': 29,
    'a1': 30,
    'p1': 31,
    'a2': 32,
    'p2': 33,
    'a3': 34,
    'p3': 35,
    'a4': 36,
    'p4': 37,
    'a5': 38,
    'p5': 39,
    'a6': 40,
    'p6': 41,
    'an': 42,
    'ab': 43,
    'ap': 44,
    'os': 45,  # C
    'g0': 46,
    'dF': 47,
    'db': 48
}

# These are other parameters that are not in defval and not in Klatt's original
# .doc file format. They are present in the .klp format.
extra_params = (
    'agc',  # automatic gain control mode (formerly the -g command line switch)
            # comma is necessary to maintain this as a single-valued tuple
)

class synthesizer(object):
    def __init__(self):
        ''' Initialize global variables as done in C code's main() function.'''
        # This section is from main().
        for idx in range(klatt_defs.NPAR):
            klatt_defs.defval[idx] = klatt_defs.cdefval[idx]
        #self.update_constant_params_from_defval()  # seems to be okay without this
        klatt_defs.initpars()

    def update_constant_params_from_defval(self, verbose=0):
        ''' Set C variables for constant params from current values in defval[]. '''
        klatt_defs.OUTSELECT = klatt_defs.defval[params_map['os']]
        klatt_defs.RANSEED = klatt_defs.defval[params_map['rs']]
        klatt_defs.NFCASC = klatt_defs.defval[params_map['nf']]
        klatt_defs.SOURCE_SELECT = klatt_defs.defval[params_map['ss']]
        klatt_defs.setlimits(verbose)

    def get_defval(self, param):
        ''' Get a parameter value from klatt_defs.defval[]. '''
        return klatt_defs.defval[params_map[param]]

    def get_constant_params(self):
        ''' Return constant params in a dict. '''
        params = {}
        for idx in range(klatt_defs.NPAR):
            param = list(params_map.keys())[list(params_map.values()).index(idx)]
            if klatt_defs.cv[idx] != klatt_defs.VARRIED:
                params[param] = klatt_defs.defval[idx]
        for param in extra_params:
            if param == 'agc':
                params[param] = klatt_defs.gain_control
        return params

    def get_varied_params(self):
        ''' Return varied params in a dict. '''
        params = {}
        for idx in range(klatt_defs.NPAR):
            param = list(params_map.keys())[list(params_map.values()).index(idx)]
            if klatt_defs.cv[idx] == klatt_defs.VARRIED:
                val = np.zeros([klatt_defs.nframes], dtype=np.int16)
                for nf in range(len(val)):
                    val[nf] = klatt_defs.pdata[idx][nf]
                params[param] = val
        return params

    def get_params(self):
        ''' Return all params in a dict. '''
        return dict(
                   list(self.get_constant_params().items()) +
                   list(self.get_varied_params().items())
        )

    def set_params(self, params=None):
        ''' Set C variables based on params. If a param has only a single value, it is treated as a constant variable. If its value is list-like it is treated as a varied parameter. '''

        for idx in range(klatt_defs.NPAR):
            klatt_defs.clearpar(idx)
        klatt_defs.ipsw = klatt_defs.MYTRUE

        num_varied = 0
        for (param, val) in params.items():
            if param in extra_params:
                if param == 'agc':
                    klatt_defs.gain_control = int(val)
            else:
                # kl_np corresponds to np in C code. We don't use 'np' here
                # because it conflicts with the usual use of 'np' for numpy.
                kl_np = params_map[param]
                if np.isscalar(val):
                    klatt_defs.defval[kl_np] = int(val)
                    self.update_constant_params_from_defval()
                else:
                    for (nf, v) in enumerate(val):
                        klatt_defs.cv[kl_np] = klatt_defs.VARRIED
                        klatt_defs.pdata[kl_np][nf] = int(v)
                    num_varied += 1
        klatt_defs.nvar = num_varied
        return self.get_params()

    def get_ms_per_frame(self):
        ''' Get the milliseconds-per-frame value. '''
        return klatt_defs.ms_frame

    def synthesize(self):
        ''' Synthesize waveform based on state of global variables in C code and return the waveform data and sample rate. '''
        klatt_defs.actonrequest('s', 1) # Assume synthesis and batch mode.
        # TODO: don't assume one channel, or disable multichannel synthesis in C code
        pywav = np.zeros([klatt_defs.nsamtot], dtype=np.int16)
        # TODO: figure out how to access klatt_defs.iwave without copying
        for idx in range(klatt_defs.nsamtot):
            pywav[idx] = klatt_defs.iwave[idx]
        return (pywav, self.get_defval('sr'))

