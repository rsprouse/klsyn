# -*- coding: utf-8 -*-
"""
Created on Tue Nov 26 14:21:50 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""

import re
import numpy as np
import klsyn.klatt_wrap as klatt_wrap

''' Functions for reading and writing .klp param files for Klatt synthesizer.'''

# A .klp file consists of three ordered sections, all optional:
#
# 1. A block of comments at the top.
# 2. A constant parameters section.
# 3. A varied parameters section.
# 
# Lines in the comments section begin with '#'.
#
# Constant parameters are provided one per line. They consist of the parameter
# name and its integer value, separated by whitespace.
#
# The varied parameters section is introduced by the line consisting of
# exactly:
#_varied_params_
# 
# This line is followed by a whitespace-delimited table of varied parameters,
# in which the columns are the parameters, and the rows are time points.
# The first line of the table is a header that defines the columns. Each
# column name defined in this header must be a valid parameter name, or the
# name must begin and end with '_', in which case the column is ignored.
#
# The primary use of the ignored column is so that human readers can include
# a time index for each of the rows for their own convenience. By default,
# klpfile() will include such an index as the '_msec_' column when writing.
# Be careful--klpfile() simply ignores the column on reading and will not
# detect inconsistencies in the time index (gaps, repetitions, misordering,
# etc.).
#
# Comments may be included on any parameter line, constant or varied. Any
# portion of a parameter line to the right of the '#' character is a
# comment. These parameter comments are returned from the read() function,
# and if these comments are supplied to write() they will be written in the
# corresponding locations.

def read(fname):
    ''' Read a .klp parameter file into a dict and return the dict. Also return comments. '''
    sep = "\s+"
    params = {}
    comments = {'header': '', 'constant': {}, 'varied': []}
    fields = None
    field_map = {}
    varparams_re = re.compile('^\s*_varied_params_\s*$')
    comment_re = re.compile('^\s*#')               # a comment line
    empty_re = re.compile('^\s*$')                 # an empty line
    eol_comment_re = re.compile('(?P<comment>\s*#.*)$')         # an end-of-line comment
    with open(fname, 'rb') as f:

        # Read header comments.
        reading_header = True
        header_comments = ''
        loc = f.tell()
        while reading_header:
            line = f.readline()
            if comment_re.search(line):
                loc = f.tell()
                header_comments += line.rstrip() + "\n"
            else:
                f.seek(loc)   # Not a comment, rewind to previous line.
                comments['header'] = header_comments
                reading_header = False

        # Read constant and varied parameters.
        reading_constparams = True
        for line in f.readlines():
            if varparams_re.search(line):
                reading_constparams = False
                continue
            elif empty_re.search(line):
                continue
            elif reading_constparams:
                comment = ''
                m = eol_comment_re.search(line)
                if m:
                    comment = m.group('comment')
                    line = eol_comment_re.sub('', line)
                (p, val) = re.split(sep, line.strip())
                params[p.strip()] = int(round(float(val.strip())))
                comments['constant'][p.strip()] = comment.rstrip()
            elif fields == None:       # reading_constparams == False
                line = eol_comment_re.sub('', line)
                fields = re.split(sep, line.strip())
                for idx,fld in enumerate(fields):
                    fld = fld.strip()
                    field_map[str(idx)] = fld
                    if fld in klatt_wrap.params_map.keys():
                        params[fld] = []
                    elif not (fld.startswith('_') and fld.endswith('_')):
                        raise Exception(
                            "Unrecognized varied parameter '{:s}'.\n".format(
                                fld)
                            )
            else:
                comment = ''
                m = eol_comment_re.search(line)
                if m:
                    comment = m.group('comment')
                    line = eol_comment_re.sub('', line)
                comments['varied'].append(comment.rstrip())
                vals = re.split(sep, line.strip())
                for idx,val in enumerate(vals):
                    val = val.strip()
                    fld = field_map[str(idx)]
                    if fld in klatt_wrap.params_map.keys():
                        params[fld].append(int(round(float(val))))
    return (params, comments)

def write(fname, synth=None, comments=None, withTimeIndex=True):
    ''' Write out params to a .klp param file. synth is a klatt_wrap synthesizer object to read params from. '''
    sep = "\t"
    with open(fname, 'wb') as f:
        # Write the header comments.
        try:
            f.write(comments['header'])
        except TypeError:     # comments == None
            pass

        # Write the constant parameters.
        for (param, val) in synth.get_constant_params().items():
            f.write("{:s}{:s}{:d}".format(param, sep, val))
            try:
                f.write(comments['constant'][param])
            except (TypeError, KeyError):
                pass
            f.write("\n")

        # Write the varied parameters.
        f.write("\n_varied_params_\n")
        vp = synth.get_varied_params()
        if withTimeIndex:
            f.write("_msec_{:s}".format(sep))
        f.write(sep.join(vp.keys()) + "\n")
        param_values = np.char.mod('%d', np.array(vp.values()).transpose())
        for idx, vals in enumerate(param_values):
            if withTimeIndex:
                f.write("{:d}{:s}".format(idx * synth.get_ms_per_frame(), sep))
            f.write(sep.join(vals))
            try:
                f.write(comments['varied'][idx])
            except (TypeError, IndexError):
                pass
            f.write("\n")
