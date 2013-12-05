#!/usr/bin/env python

# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 14:40:35 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""
# This is an implementation of the Klatt synthesizer suitable for running
# in batch mode.

import sys
import scipy.io.wavfile
import klsyn.klatt_wrap as klatt_wrap
from klsyn.klpfile import klpfile

if __name__ == '__main__':
    ''' Run the Klatt synthesizer, produce output .wav and write out complete .klp parameter file.'''
# TODO: check command line arguments
    synth = klatt_wrap.synthesizer()
    params = klpfile().read(sys.argv[1] + '.klp')
    synth.set_params(params)
    (d,rate) = synth.synthesize()
    scipy.io.wavfile.write(sys.argv[1] + '.wav', rate, d)
    klpfile().write(sys.argv[1] + '.wav.klp', synth=synth)
