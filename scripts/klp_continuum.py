#!/usr/bin/env python

# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 14:40:35 2013

@author: Keith Johnson (keithjohnson@berkeley.edu)

this is a command line implementation with a pygame play command
"""
import os, sys, re, math, platform
import scipy.io.wavfile
import klsyn.klatt_wrap as kw
import klsyn.klpfile
import subprocess

        
def playaudio(filename):
    
    if 'Windows' in platform.system():
        p = subprocess.Popen(["sox", "-q", filename, "-d"],shell=True)
    else:
        p = subprocess.Popen(["sox", "-q", filename, "-d"],shell=False)
    p.wait()  
    

Usage = 'klp_continuum (klpfile1) (klpfile2)'
nsteps = 7   # the number of files will be nsteps + 1

if __name__ == '__main__':

    print "klp_continnum: make a continuum between two .klp files"
    print "  - time warp if the files are different durations"
    print "  - time warp segments if there are matching comments in the .klp files"

    if len(sys.argv) > 1:
        pfile = sys.argv[1]
        fname, fext = os.path.splitext(pfile)
        if fext != '.klp':
            print "{} is not a .klp file. ".format(pfile)
        else:
            file1 = pfile
        if len(sys.argv)>2:
            pfile = sys.argv[2]
            fname, fext = os.path.splitext(pfile)
            if fext != '.klp':
                print "{} is not a .klp file. ".format(pfile)
            else:
                file2 = pfile
        else:
            file2 = raw_input("Input second .klp file: ").strip()
            
        
    else:
        file1 = raw_input("Input first .klp file ").strip()
        file2 = raw_input("Input second .klp file ").strip()
    
    try:
        (params1, comments1) = klsyn.klpfile.read(file1)
    except:
        exit()
    try:
        (params2, comments2) = klsyn.klpfile.read(file2)
    except:
        exit()
        
    if params1.get('ui') != params2.get('ui'):
        print 'need to have <ui> match in the two files'
        exit()
    
    fname1, ext = os.path.splitext(file1)
    fname2, ext = os.path.splitext(file2)
    
    
    # from the comments, extract labels that mark segments for interpoloation.
    # we will time align on these 
    labels1 = comments1.get('varied')
    labels2 = comments2.get('varied')
    segments1 = []
    segments2 = []
    segments1.append(0)
    segments2.append(0)
    j=0
    
    for i in range(0,len(labels1)):
        if labels1[i].strip() != '':
            while j < len(labels2)-1 and labels2[j].strip() != labels1[i].strip():
                j += 1
            if labels1[i].strip() != labels2[j].strip(): 
                print 'expected to find a label {} in {}'.format(labels1[i],file2)
            else:
                segments1.append(i)
                segments2.append(j)
    segments1.append(len(labels1))
    segments2.append(len(labels2))    
    comments1['varied']= ''  # wipe out labels on output
    comments1['header']= '#  produced by klp_continuum from ' + file1 + ' and '+ file2 + '\n'
    
    for s in range(0,nsteps):
        s_prop = float(s)/(nsteps-1)
        params = {}
        for fld1 in params1:   # we assume that the files have the same fields
            v1 = params1.get(fld1)
            v2 = params2.get(fld1)
        
            if isinstance(v1, list) or isinstance(v2, list):
                params[fld1] =[]
            
                for seg in range(1,len(segments1)):
                    d1 = float(segments1[seg]-segments1[seg-1])
                    d2 = float(segments2[seg]-segments2[seg-1])
                    nframes = int(round(d1+s_prop*(d2-d1)))                 

                    for i in range(0, nframes):
                        idx1 = segments1[seg-1] + int(i*(d1/nframes))
                        idx2 = segments2[seg-1] + int(i*(d2/nframes))                  
                    
                        if isinstance(v1,list):
                            if isinstance(v2,list):
                                val = v1[idx1] + s_prop*(v2[idx2]-v1[idx1])
                                params[fld1].append(int(round(val)))
                            else:
                                val = v1[idx1] + s_prop*(v2-v1[idx1])
                                params[fld1].append(int(round(val)))
                        else:
                            val = v1 + s_prop*(v2[idx2]-v1)
                            params[fld1].append(int(round(val)))
                        i += 1
                params['du'] = len(params[fld1])*params1.get('ui')
            else:
                myv = v1 + s_prop*(v2-v1)
                params[fld1] = int(round(myv))
            
        text1 = '#  produced by klp_continuum from ' + file1 + ' and '+ file2 + '\n'
        text2 = '#     Step '+ str(s+1) + ' of '+str(nsteps)+'\n'        
        comments1['header']= text1 + text2
        
        fname = "{}_{}_{}".format(fname1,fname2,s+1)
        synth = kw.synthesizer()
        synth.set_params(params)
        (d,rate) = synth.synthesize()
        scipy.io.wavfile.write(fname + '.wav', rate, d)
        klsyn.klpfile.write(fname + '.klp', synth=synth, comments=comments1)
        playaudio(fname + '.wav')
        print "\n--Files {} and {} were saved.".format(fname+'.wav',fname+'.klp')

