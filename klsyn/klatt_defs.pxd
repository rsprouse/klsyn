# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 14:44:19 2013

@author: Ronald L. Sprouse (ronald@berkeley.edu)
"""

cdef extern from '../c/klsyn.h':
    int NPAR             # 49
    int MAX_VAR_PARS     # 49
    int FIXED           # 'C'
    int VARIABLE        # 'v'
    int VARRIED         # 'V'
    int NOVICE          # 'n'
    int MYTRUE          # 't'
    int MAX_FRAMES       # 10000 (was 200 in original c code)
    int OUTSELECT	  #spkrdef[0]
    int SAMRAT		#  spkrdef[1]
    int NSAMP_PER_FRAME   #spkrdef[2]     /* Number of samples per frame */
    int RANSEED	#	  spkrdef[3]
    int NFCASC	#	  spkrdef[4]
    int SOURCE_SELECT	#  spkrdef[5]
    int batch
    int NPAR
    int *defval
    int *cdefval
    int *cv  # really char
    int **pdata

    # c functions
    void setlimits(int) 
    void initpars() 
    void actonrequest(char ,int)
    void clearpar(int np)

cdef extern from '../c/klsyn.c':
    int ipsw
    int np
    int npars
    int nvar
    int totdur
    int ms_frame
    int nframes
    int gain_control
    int request  # really char
    char *firstname
    int nsamtot
    int *iwave

cdef extern from '../c/parwv.c':
    pass

cdef extern from '../c/parwvt.h':
    pass

