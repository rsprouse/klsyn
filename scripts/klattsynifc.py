#!/usr/bin/env python

# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 14:40:35 2013

@author: Keith Johnson
"""
# This is an implementation of the Klatt synthesizer suitable for running
# in batch mode.

import os, sys, re, math
import scipy.io.wavfile
import klsyn.klatt_wrap as kw
import klsyn.klpfile

def ifc2klp(fname,start,end):
    ''' read an ifcformant formant traces file and convert to params for the klatt synthesizer '''
    sep = "\s+"
    fields = None
    field_map = {}
    params = {}
    nonklatt_params={}
    comments = {'header': '#  produced by ifc2klp', 'constant': {}, 'varied': []}

    with open(fname, 'rb') as f:
        # Read header comments.
        reading_header = True
        firsttime = True
        for line in f:
            if reading_header:
                fields = re.split(sep, line.strip())
                for idx,fld in enumerate(fields):
                    fld = fld.strip()
                    if fld == 'f1':
                        fld = 'F1'
                    if fld == 'f2':
                        fld = 'F2'
                    if fld == 'f3':
                        fld = 'F3'
                    if fld == 'f4':
                        fld = 'F4'

                    field_map[str(idx)] = fld
                    if fld in kw.params_map.keys():
                        params[fld] = []
                    else:
                        nonklatt_params[fld]=[]
                reading_header = False
            else:
                vals = re.split(sep, line.strip())
                for idx,val in enumerate(vals):
                    val = float(val.strip())
                    fld = field_map[str(idx)]
                    if fld == 'sec':
                        if val < start:
                            break
                        if val > end:
                            break
                        val = (val*1000)   #put time into ms
                        if firsttime:
                            subtracttime = val
                            firsttime=False
                        val = val - subtracttime
                        nonklatt_params[fld].append(int(round(val)))
                            
                    elif fld in kw.params_map.keys():
                        params[fld].append(int(round(val)))
                    elif fld == 'rms': 
                        val = 20*math.log10(val)
                        nonklatt_params[fld].append(int(round(val)))
                        
    params['du'] = nonklatt_params['sec'][len(nonklatt_params['sec'])-1]
    params['ui'] = nonklatt_params['sec'][1]-nonklatt_params['sec'][0]
    params['nf'] = 4

    params['av'] = []
    params['af'] = []
    rms_change = 60 - max(nonklatt_params['rms'])
    for i in range(0,len(nonklatt_params['rms'])-1):
        scaledrms = max(0,nonklatt_params['rms'][i]+rms_change)
        if params['f0'][i]>0:
            params['av'].append(scaledrms)
            params['af'].append(0)
        else:
            params['av'].append(0)
            params['af'].append(scaledrms)

   # params['_msec_'] = nonklatt_params['sec']
                        
    return (params, comments)

Usage = 'klattsyn ifcparamfile [start] [end]'

if __name__ == '__main__':
    ''' Run the Klatt synthesizer on an ifcformant output file, produce output .wav and write out complete .klp parameter file.'''
    try:
        sys.argv[1] != None
    except:
        raise Exception('Usage: {:s}'.format(Usage))
    try:
        start = sys.argv[2]
    except:
        start = 0.0
    try:
        end = sys.argv[3]
    except:
        end = float('inf')
    print "start is %0.4f" % start
    print "end is %0.4f" % end

    pfile = sys.argv[1]
    print "pfile is %s" % pfile
    fname, fext = os.path.splitext(pfile)
    synth = kw.synthesizer()
    
    (params,comments) = ifc2klp(pfile,start,end)
    fname = fname + "_klp"
    synth.set_params(params)
    (d,rate) = synth.synthesize()
    scipy.io.wavfile.write(fname + '.wav', rate, d)
    klsyn.klpfile.write(fname + '.wav.klp', synth=synth, comments=comments)
