# -*- coding: utf-8 -*-
"""
Created on Tue Nov 26 14:21:50 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""

import re
import numpy as np
import klsyn.klatt_wrap as klatt_wrap

class klpfile(object):
    ''' Class for reading and writing .klp param files for Klatt synthesizer.'''
    
    def __init__(self, *args, **kwargs):
        super(klpfile, self).__init__(*args, **kwargs)

    def read(self, fname):
        ''' Read a .klp parameter file into a dict and return the dict. '''
        params = {}
        sep = '\t'
        fields = None
        field_map = {}
        nf = 0
        varparams_re = re.compile('^\s*__varied_params__\s*$')
        comment_re = re.compile('^\s*#')               # a comment line
        empty_re = re.compile('^\s$')                  # an empty line
        eol_comment_re = re.compile('\s*#.*$')         # an end-of-line comment
        with open(fname, 'rb') as f:
            reading_constparams = True
            for line in f.readlines():
                if varparams_re.search(line):
                    reading_constparams = False
                    continue
                elif comment_re.search(line):
                    continue
                elif empty_re.search(line):
                    continue
                elif reading_constparams:
                    line = eol_comment_re.sub('', line)
                    (p, val) = line.split(sep)
                    params[p.strip()] = val.strip()
                elif fields == None:       # reading_constparams == False
                    line = eol_comment_re.sub('', line)
                    # TODO: error checking
                    fields = line.strip().split(sep)
                    for idx,fld in enumerate(fields):
                        fld = fld.strip()
                        field_map[str(idx)] = fld
                        if fld in klatt_wrap.params_map.keys():
                            params[fld] = []
                else:                      # varparams line
                    line = eol_comment_re.sub('', line)
                    vals = line.split(sep)
                    for idx,val in enumerate(vals):
                        val = val.strip()
                        fld = field_map[str(idx)]
                        if fld in klatt_wrap.params_map.keys():
                            params[fld].append(int(val))
        return params

    def write(self, fname, synth=None):
        ''' Write out params to a .klp param file. synth is a klatt_wrap synthesizer object to read params from. '''
        with open(fname, 'wb') as f:
            varied = []
            for (param, val) in synth.get_constant_params().items():
                print "writing param ", param, " val ", val
                f.write("{:s}\t{:d}\n".format(param, val))
            f.write("\n__varied_params__\n")
            vp = synth.get_varied_params()
            f.write("\t".join(vp.keys()) + "\n")
            for vals in np.char.mod('%d', np.array(vp.values()).transpose()):
                f.write("\t".join(vals) + "\n")
