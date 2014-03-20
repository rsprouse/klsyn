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

def interactive(params,comments,fname):
    sep = "\s+"
    ui = 5   # hard code 5ms frame size

    print "Klattsyn Interactive"
    print "     FILE "
    print "       <o> to open a .klp or .fb file"
    print "       <s> to save your results in .klp and .wav files."
    print "       <p> to play the synthesized speech."
    print "       <q> to quit without saving anything.\n"
    print "     EDIT"
    print "       <c> to input a constant parameter."
    print "       <v> to input a varied parameter trajectory."
    print "       <t> to show a table of parameters."

    
    if 'du' not in params.keys():  # set defaults if not read from file
        params['du']=500
        params['ui']=ui
    else:
        ui = params['ui']
    
    while True:
        cmd = raw_input('\nEnter \'o\', \'s\', \'p\', \'q\',    \'c\', \'v\', or \'t\': ').strip()

        if cmd == 'c':
            fld = raw_input("\twhich parameter? ").strip()
            if fld not in kw.params_map.keys() or fld == 'ui':  # ignore unknown parameter or UI
                continue
            val = int(raw_input("\twhat value should it be? ").strip())
            params[fld]=val

        if cmd == 's':
            temp = raw_input('\tWhat should the first name of the .wav file be? ').strip()
            try:
                fname, fext = os.path.splitext(temp)
            except ValueError:
                fname = temp
            synth = kw.synthesizer()
            synth.set_params(params)
            (d,rate) = synth.synthesize()
            scipy.io.wavfile.write(fname + '.wav', rate, d)
            klsyn.klpfile.write(fname + '.klp', synth=synth, comments=comments)
            print "\tFiles {} and {} were saved.".format(fname+'.wav',fname+'.klp')

        if cmd == 'q':
            return 'finished'

        if cmd == 'p':
            synth = kw.synthesizer()
            synth.set_params(params)
            (d,rate) = synth.synthesize()
            scipy.io.wavfile.write('klattsyn_temp.wav', rate, d)
            playaudio('klattsyn_temp.wav')
            print "\n"  

        if cmd == 'o':
            pfile = raw_input("\tenter the file name: ").strip()
            fname, fext = os.path.splitext(pfile)
        
            try:
                f = open(pfile, 'r')
            except:
                print "\tdidn't find file: {}".format(pfile)
                continue
            
            d=f.read()
            f.close()
        
            if re.search('\s*_varied_params_\s*', d, re.MULTILINE) != None:
                (params, comments) = klsyn.klpfile.read(pfile)

            elif (re.match('^sec*.',d) != None):
                start = 0
                end = -1
                start = float(raw_input("\treading an ifcformants file - start time? ").strip())
                end = float(raw_input("\treading an ifcformants file - end time? ").strip())

                (params,comments) = ifc2klp(pfile,start,end)
                fname = fname + "_klp"
            else:
                print "\tthe file format of {} was not recognized.".format(pfile)
                continue
            print "\tfinished reading the file {}".format(pfile)
        
        if cmd == 'v':
            fld = raw_input("\tenter trajectory, which paramter? ").strip()
            if fld not in kw.params_map.keys():
                continue
            if fld not in params.keys() or not isinstance(params[fld],list):
                params[fld] = []
            print "\tenter time|value pairs like this \'0 60\'. Empty line terminates."
            ot=ov=-1
            while True:
                time_val = raw_input("\ttime|value pair: ").strip()
                if time_val == "":
                    break
                try:
                    (time,val) = re.split(sep,time_val)
                except ValueError:
                    continue
                if time < 0:
                    print "time can't be less than zero"
                    continue
                if int(time) >= params.get('du'):
                    maxtime = params.get('du')-ui
                    print "\t{} is the current maximum time value.".format(maxtime)
                    continue
                if (ot == -1):  # first time point
                    ot = int(ui*round(float(time)/ui))  # force it to multiple of ui
                    ov = float(val)
                    continue
                t = int(ui*round(float(time)/ui))  # force it to multiple of ui
                v = float(val)

                if t <= ot:      # see if we are going forwards or backwards
                    print "enter sucessively larger time values."
                    continue
                    
                dv = v-ov
                for t1 in range(ot,t+ui,ui):
                    idx = int(t1/ui)
                    newv = int(round(ov + ((dv*(t1-ot))/(t-ot))))
                    if (idx > len(params[fld])-1):
                        while (idx > len(params[fld])):
                            params[fld].append(0)
                        params[fld].append(newv)
                    else:
                        oldval = params[fld].pop(idx)                  
                        params[fld].insert(idx,newv)
                ov = v
                ot = t
                        
        if cmd == 't':
            show_params(params)
                
def show_params(params):
        ncol = 0
        for fld,val in sorted(params.items()):
            if not isinstance(val,list):
                sys.stdout.write("{} {}".format(fld,val))
                if ncol == 5:
                    sys.stdout.write("\n")
                    ncol=0
                else:
                    sys.stdout.write("\t")
                    ncol += 1
        for fld,val in sorted(params.items()):
            if isinstance(val,list):
                print "{}: {}".format(fld,val)
                
def ifc2klp(fname,start,end):
    ''' read an ifcformant formant traces file and convert to params for the klatt synthesizer '''
    sep = "\s+"
    fields = None
    field_map = {}
    params = {}
    nonklatt_params={}
    comments = {'header': '#  produced by ifc2klp\n', 'constant': {}, 'varied': []}

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
                        if val<start:
                            break
                        if end != -1 and val > end:
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
    
    
def playaudio(filename):
    
    if 'Windows' in platform.system():
        p = subprocess.Popen(["sox", "-q", filename, "-d"],shell=True)
    else:
        p = subprocess.Popen(["sox", "-q", filename, "-d"],shell=False)
    p.wait()  


#    import pygame
#    pygame.mixer.init()
#    sound = pygame.mixer.Sound(filename)
#    channel = sound.play()
#    while channel.get_busy():
#        pygame.time.delay(100)
    

Usage = 'klattsyn (paramfile) (start) (end) '

if __name__ == '__main__':
    ''' Run the Klatt synthesizer, produce output .wav and write out complete .klp parameter file.'''

    if len(sys.argv) > 1:
        pfile = sys.argv[1]
        fname, fext = os.path.splitext(pfile)
        
        try:
            f = open(pfile, 'r')
        except:
            print "didn't find file: {}".format(pfile)
            exit()
            
        d=f.read()
        f.close()
        
        if re.search('\s*_varied_params_\s*', d, re.MULTILINE) != None:
            (params, comments) = klsyn.klpfile.read(pfile)

        else:
            start = 0
            end = -1
            
            if len(sys.argv)==3: 
                start = float(sys.argv[2])  
            if len(sys.argv)==4:
                start = float(sys.argv[2])
                end = float(sys.argv[3])

            (params,comments) = ifc2klp(pfile,start,end)
            fname = fname + "_klp"
        
        print "finished reading the file {}".format(pfile)
        result = interactive(params,comments,fname)        
        
    else:        
        comments = {'header': '# produced by interactive mode\n', 'constant': {}, 'varied':[]}
        params = {}
        fname = "klatt_temp"
        result = interactive(params,comments,fname)
        
