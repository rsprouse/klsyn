# -*- coding: utf-8 -*-
"""
Created on Tue Nov 26 14:21:50 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""

import re
import klsyn.klatt_wrap as klatt_wrap

''' Function for reading .doc param files for Klatt synthesizer. These are the old-style parameter files in the original C code. '''

def read(fname):
    ''' Read a .doc parameter file into a dict and return the dict. '''
    sep = "\s+"
    params = {}
    fields = None
    field_map = {}
    empty_re = re.compile('^\s*$')
    varparams_re = re.compile('^\s*Varied Parameters:\s*$')
    dashes_re = re.compile('^\s*------------------------') # partial line before constant parameters
    with open(fname, 'r') as f:
        reading_constparams = False
        reading_varparams = False
        for line in f.readlines():
            if not reading_constparams and not reading_varparams:
                if dashes_re.search(line):
                    reading_constparams = True
                elif varparams_re.search(line):
                    reading_varparams = True
                continue
            elif reading_constparams:
                if empty_re.search(line):
                    reading_constparams = False
                else:
                    spl = re.split(sep, line.strip())
                    params[spl[0]] = int(spl[3])
                    try:    # Read second set of parameter/value on the line, if it exists
                        params[spl[5]] = int(spl[8])
                    except IndexError:
                        pass
            # Below here, reading_varparams == True
            elif fields == None:
                if empty_re.search(line):
                    continue
                fields = re.split(sep, line.strip())
                for idx,fld in enumerate(fields):
                    fld = fld.strip()
                    field_map[str(idx)] = fld
                    if fld in klatt_wrap.params_map.keys():
                        params[fld] = []
                    elif not fld == 'time':
                        raise Exception(
                            "Unrecognized varied parameter '{:s}'.\n".format(
                                fld)
                            )
            else:
                if empty_re.search(line):
                    continue
                vals = re.split(sep, line.strip())
                for idx,val in enumerate(vals):
                    val = val.strip()
                    fld = field_map[str(idx)]
                    if fld in klatt_wrap.params_map.keys():
                        params[fld].append(int(val))
    return params

