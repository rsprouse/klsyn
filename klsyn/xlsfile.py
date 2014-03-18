# -*- coding: utf-8 -*-
"""
Created on Tue Nov 26 14:21:50 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""

import re
import numpy as np
import klsyn.klatt_wrap as klatt_wrap
import xlrd
import xlwt

''' Functions for reading and writing .xls param files for Klatt synthesizer.'''

# A .xls file synthesizer format follows the .klp format. Lines in the .klp
# format correspond to rows in the .xls format, and .klp columns correspond
# to .xls columns.

# Comments are preceded by the '#' character and may not appear in a cell
# that contains a parameter name or parameter value. A row that begins with
# a '#' character in its first cell will be interpreted only as a comment
# row. Other comments may appear in a column immediately to the right of
# the cells containing parameter names and values.

def read(fname):
    ''' Read a .xls parameter file into a dict and return the dict. Also return comments. '''
    sep = "\s+"
    params = {}
    comments = {'header': [], 'constant': {}, 'varied': []}
    fields = None
    field_map = {}
    varparams_re = re.compile('^\s*_varied_params_\s*$')
    comment_re = re.compile('^\s*#')               # a comment line
    empty_re = re.compile('^\s*$')                 # an empty line
    eol_comment_re = re.compile('(?P<comment>\s*#.*)$')         # an end-of-line comment

    wb = xlrd.open_workbook(fname)
    ws = wb.sheet_by_index(0)
    reading_header = True
    reading_constparams = True
    header_comments = []
    for ridx in range(ws.nrows - 1):
        cells = ws.row(ridx)
        if reading_header:
            if comment_re.search(cells[0].value):
                header_comments.append(cells[0].value.rstrip())
                continue
            else:
                comments['header'] = header_comments
                reading_header = False

        if varparams_re.search(str(cells[0].value)):
            reading_constparams = False
            continue
        elif empty_re.search(str(cells[0].value)):
            continue
        elif reading_constparams:
            comment = ''
            m = eol_comment_re.search(cells[2].value)
            if m:
                comment = m.group('comment')
            p = cells[0].value
            val = cells[1].value
            params[p.strip()] = int(round(float(val)))
            comments['constant'][p.strip()] = comment.rstrip()
        elif fields == None:       # reading_constparams == False
            fields = ws.row_values(ridx)
            for idx,fld in enumerate(fields):
                fld = fld.strip()
                field_map[str(idx)] = fld
                if fld in klatt_wrap.params_map.keys():
                    params[fld] = []
                elif not (fld == '' or fld.startswith('_') and fld.endswith('_')):
                    raise Exception(
                        "Unrecognized varied parameter '{:s}'.\n".format(
                            fld)
                        )
        else:
            comment = ''
            try:
                m = eol_comment_re.search(str(cells[len(fields)].value))
                if m:
                    comment = m.group('comment')
            except:
                pass
            comments['varied'].append(comment.rstrip())
            vals = ws.row_values(ridx)
            for idx,val in enumerate(vals):
                fld = field_map[str(idx)]
                if fld in klatt_wrap.params_map.keys():
                    params[fld].append(int(round(float(val))))
    return (params, comments)


