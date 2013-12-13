#!/usr/bin/env python

# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 14:40:35 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""
# Convert Klatt synthesizer .doc parameter files to .klp files.

import os, sys
import klsyn.klatt_wrap as klatt_wrap
import klsyn.docfile
import klsyn.klpfile

Usage = 'doc2klp.py docfile ... [docfileN]'

if __name__ == '__main__':
    ''' Convert Klatt synthesizer .doc parameter files to .klp files. '''
    try:
        sys.argv[1] != None
    except:
        raise Exception('Usage: {:s}'.format(Usage))

    for dfile in sys.argv[1:]:
        fname, fext = os.path.splitext(dfile)
        synth = klatt_wrap.synthesizer()
        params = klsyn.docfile.read(dfile)
        synth.set_params(params)
        klsyn.klpfile.write(fname + '.klp', synth=synth)
