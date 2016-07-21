#!/usr/bin/env python

# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 14:40:35 2013

@author: Keith Johnson (keithjohnson@berkeley.edu)
"""

# This is an implementation of the Klatt synthesizer with a gui interface.

import wxversion
wxversion.ensureMinimal('2.8')
import os,sys, subprocess, platform, math, re
import numpy as np
import wx
import matplotlib
matplotlib.use('WXAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas

import scipy.io.wavfile
import klsyn.klatt_wrap as kw
import klsyn.klpfile

param_info = {
'sr':	{'max':	22050, 'min':	5000, 'default': 10000, 'name':'Sampling Rate'},
'nf':	{'max':	8, 'min':1, 'default':  5, 'name':'Number of cascade formants'},
'du':	{'max':	5000	, 'min':30 , 'default': 500, 'name':'Sound file duration'},
'ss':	{'max':2	, 'min':1, 'default': 2, 'name':'Source select'},
'ui':	{'max':20	, 'min':1, 'default': 5, 'name':'Update interval'},
'rs':	{'max':99	, 'min':1	, 'default':1, 'name':'Random Seed'},
'f0':	{'max':	500	, 'min':50	, 'default':100, 'name':'Fundamental frequency of voicing'},
'av':	{'max':	80	, 'min':0	, 'default':60, 'name':'Amplitude of voicing'},
'F1':	{'max':	1300	, 'min':180	, 'default':350, 'name':'First Formant'},
'b1':	{'max':1000	, 'min':30	, 'default':60, 'name':'Bandwidth of F1'},
'F2':	{'max':3000	, 'min':550	, 'default':850, 'name':'Second Formant'},
'b2':	{'max':	1000	, 'min':40, 'default':	70, 'name':'Bandwidth of F2'},
'F3':	{'max':	4800	, 'min':1200, 'default':	2500, 'name':'Third Formant'},
'b3':	{'max':	1000	, 'min':60	, 'default':150, 'name':'Bandwidth of F3'},
'F4':	{'max':	4990	, 'min':2400, 'default':	3250, 'name':'Fourth Formant'},
'b4':	{'max':	1000	, 'min':100	, 'default':200, 'name':'Bandwidth of F4'},
'F5':	{'max':	4990	, 'min':3000, 'default':	3700, 'name':'Fifth Formant'},
'b5':	{'max':	1500	, 'min':100	, 'default':200, 'name':'Bandwidth of F5'},
'f6':	{'max':	4990	, 'min':3000, 'default':	4990, 'name':'Sixth formant'},
'b6':	{'max':	4000	, 'min':100	, 'default':500, 'name':'Bandwidth of f6'},
'fz':	{'max':	800	, 'min':180	, 'default':280, 'name':'Nasal zero frequency'},
'bz':	{'max':	1000	, 'min':40	, 'default':90, 'name':'Nasal zero bandwidth'},
'fp':	{'max':	500	, 'min':180	, 'default':280, 'name':'Nasal pole frequency'},
'bp':	{'max':	1000	, 'min':40	, 'default':90, 'name':'Nasal pole bandwidth'},
'ah':	{'max':	80	, 'min':0	, 'default':0, 'name':'Amplitude of aspiration'},
'oq':	{'max':	80	, 'min':10	, 'default':50, 'name':'Open Quotient'},
'at':	{'max':	80	, 'min':0	, 'default':0, 'name':'Amplitude of turbulence'},
'tl':	{'max':	34	, 'min':0	, 'default':0, 'name':'Voice spectral tilt'},
'af':	{'max':	80	, 'min':0	, 'default':0, 'name':'Amplitude of frication'},
'sk':	{'max':	100	, 'min':0	, 'default':0, 'name':'Voice skew'},
'a1':	{'max':80	, 'min':0 , 'default':0, 'name':'Amplitude of F1'},
'p1':	{'max':1000	, 'min':30	, 'default':80, 'name':'Bandwidth of F1'},
'a2':	{'max':80	, 'min':0	, 'default':0, 'name':'Amplitude of F2'},
'p2':	{'max':	1000	, 'min':40, 'default':	200, 'name':'Bandwidth of F2'},
'a3':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of F3'},
'p3':	{'max':	1000	, 'min':60, 'default':	350, 'name':'Bandwidth of F3'},
'a4':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of F4'},
'p4':	{'max':	1000	, 'min':100, 'default':	500, 'name':'Bandwidth of F4'},
'a5':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of F5'},
'p5':	{'max':	1500	, 'min':100, 'default':	600, 'name':'Bandwidth of F5'},
'a6':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of F6'},
'p6':	{'max':	4000	, 'min':100, 'default':	800, 'name':'Bandwidth of F6'},
'an':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of nasal formant'},
'ab':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of the bypass path'},
'ap':	{'max':	80	, 'min':0, 'default':	0, 'name':'Amplitude of voicing'},
'os':	{'max':	20	, 'min':0, 'default':	0, 'name':'Output Select'},
'g0':	{'max':	80	, 'min':0, 'default':	60, 'name':'Overall gain'},
'dF':	{'max':	100	, 'min':0, 'default':	0, 'name':'Delta of formant frequencies'},
'db':	{'max':	400	, 'min':0, 'default':	0, 'name':'Dela of formant bandwidths'}     }

param_text = {
'sr':	"""The constant 'sr', "sampling rate", is the number of output samples computed per second of synthetic speech.  It is suggested that the default value of 10,000 samples/sec not be changed unless the user understands the digital signal processing implications of such a change (for example, if only 'sr' is increased, the spectrum of the synthetic speech will tilt down). However, if a sampling rate of 16,000 samples/sec is desired, one can change 'nf', the number of formants in the cascade branch, to 8 and obtain synthesis that is nearly identical below 5 kHz to that generated at 10,000 samples/sec (see description below of parameter 'nf').""",
'nf':	"""The constant 'nf', "number of formants in cascade vocal tract", specifies how many formants, counting from F1 up to a maximum of F8, are actually in the cascade vocal tract.  The default value is 5, which is an appropriate number if the sampling rate is 10,000 samples/sec and the speaker has a vocal tract length of 17 cm.  (i.e. the average spacing between formants will then be 1000 Hz).

	If the speaker that you are trying to model has a vocal tract length significantly different from 17 cm, or if the 'sr' sampling rate parameter has been changed, you may wish to modify 'nf'.  For example, to model a typical female voice with a vocal tract length about 20% shorter than the average male, one would set 'nf' to four.

	If the sampling rate is changed to 16,000 samples/sec, then a male voice should have 8 formants in the frequency range from 0 to 8 kHz, and thus 'nf' should be set to 8.  Only the lower 6 formant frequencies and bandwidths are settable by the user; the frequency and bandwidth of the seventh and eighth formants are fixed at F7=6500, B7=500, F8=7500, B8=600.  The parallel vocal tract has only 6 formants, so that one would have to move F6 up in frequency to generate noise spectra with peaks above the default value of F6=4990 Hz when 'sr' is increased.

	It should be clear that 'nf' only crudely approximates variations in vocal tract length.  If, for example, a speaker had a vocal tract length 10% shorter than the typical male, one would have to use five formants in the cascade branch, setting the higher formants appropriately higher in frequency, and then use the 'tl' tilt parameter to achieve the correct general spectral tilt for this voice.""", 
'du':	"""The constant 'du', "duration", of the utterance to be synthesized, is the number of msec from beginning to end of the current synthetic utterance, including at least 25 msec at the end to allow the waveform to decay naturally after you have turned off all the sound sources.

	The current maximum value for 'du' is 5000 (five seconds). (Actually, the maximum utterance duration is 1000 frames times ui). The specified value for 'du' will be rounded up to the nearest multiple of 'ui', the number of msec in a parameter update time interval.""",

'ss':	"""The constant 'ss', "source switch", is a switch that determines which of two voicing source waveforms is used for synthesis.  The default value, 1, causes a low-pass filtered impulse train to be generated, while the value 2 causes a more natural waveform with a definite sharp closing time to be invoked.  Each has its own set of advantages and disadvantages.

      See Klatt & Klatt (1990) for more discussion of the voices in klsyn.""",

'ui':	"""The constant 'ui', "update interval", is the number of msec of waveform generated between times when parameter values are updated. The default value of 5 ms is frequent enough to mimic most rapid parameter changes that occur in speech (in fact, 10 ms updates may be often enough).  Under special circumstances, a shorter update interval, e.g. 1 ms, might be desirable, but note the qualification given in the next paragraph.

	Parameters involved in generating the glottal source waveform ('f0' 'av' 'oq' 'tl' 'sk') are not changed at the exact time specified by the update interval.  Instead, their change in value is delayed to the next waveform sample at which glottal opening occurs.  For low values of fundamental frequency, this delay may be as much as 10 ms (the average delay is 5 ms when 'f0' is 100 Hz, and 2.5 ms when f0=200 Hz).

	If this were not done, it would be as if spurious excitation occurred at the update rate, resulting in perceptible auditory distortion. (The fact that formant frequencies and bandwidths change at the update time means that small waveform distortions synchronized to the update rate are unavoidable.)  Delaying changes to the voicing source control parameters in order to synchronize them with the time of primary excitation of the vocal tract both removes the update interval periodicity of the distortions, and better hides them under the signal.""",

'rs':	"""The constant 'rs', "random seed", is the seed value given to the random number generator routine.  Any number from 0 to 99999 can be specified.  For each, you will get a quite different random number sequence (different frication and aspiration noises from those used to generate the previous stimuli).

	On the other hand, stimuli all generated with the same value for 'rs' will have identical frication source and aspiration source waveforms. This is sometimes desirable if stimuli on a continuum are not to differ due to random fluctuations in e.g. a burst of frication noise.""",

'f0':	"""The variable 'f0', "fundamental frequency", is the rate at which the vocal folds are currently vibrating in Hz.  {In the original version of this synthesizer f0 was specified in 1/10 Hz units, i.e. 100Hz = 1000. The additional accuracy resulting from a specification of fundamental frequency to 0.1 Hz adds some naturalness to a slowly changing pitch glide.  I changed this because it seemed like overkill, and was a pain to remember to use 1/10 Hz, but if anyone notices some glitchiness, it can be changed back.}

	A new fundamental period is computed each time the vocal folds begin to open.  The value of 'f0' existing at that time instant is used to determine the new period. Several other parameters of the voicing source ('av', 'no', 'tl', 'sk') change value at this time rather than changing at the nominal update time -- otherwise discontinuities could occur in the voicing waveform.

	The fundamental period is quantized in a digital speech synthesizer. In this simulation, the period (time between instants when glottal opening occurs) is quantized to increments of 1/40000 sec. This means that at 100 Hz, 'f0' is effectively specified in 0.25 Hz steps (0.25% quantization error), while at 200 Hz, 'f0' is quantized in 0.5 Hz steps (still a 0.25% quantization error in 'f0').  This accuracy is necessary to avoid perceptible "staircase pitch" problems for slowly gliding 'f0' in the higher pitch ranges; it is achieved by running the glottal source simulation at a sampling rate four times that specified by 'sr', and lowpass/downsampling this waveform before sending it to the vocal tract model.""", 

'av':	"""The variable 'av, "amplitude of voicing" is the amplitude in dB of the voicing source waveform sent through the cascade vocal tract.  A value of 0 dB turns off (zeros) the signal.  A value of about 60 dB produces a level for vowel synthesis that is close to the maximum non-overloading level; such values should be used to keep the signal in the higher-order bits of the digital-to-analog converter.

	The synthesizer does not necessarily turn voicing on and off at exactly the time specified by the 'av' time function.  The effect of a change in 'av' is delayed until the instant of the next glottal waveform opening. If the natural source, 'ss'=2, is used, the primary excitation of the vocal tract actually begins even later, at glottal closure some 'oq' percent of the voicing period following the time of glottal opening.

	If 'av' is suddenly turned off, no more glottal pulses will be issued, and the vocal tract response to the previous pulse will continue to die out, taking 10 to 20 msec to become totally inaudible.

	If 'av' is suddenly turned ON, and you wish a glottal pulse to be issued at exactly that time, it is necessary to have set 'f0' to zero for a period of time prior to this event, and to turn 'f0' on simultaneous with the time that 'av' is turned on.  This procedure should be followed in order to specify voice onset time for a plosive as an exact number of update intervals later than burst onset.""",

('F1', 'F2', 'F3', 'F4', 'F5', 'f6') : """The "formant frequency" variables determine the frequency in Hz of up to six resonators of the cascade vocal tract model, and of the frequency in Hz of each of six additional parallel formant resonators.  Normally, the cascade branch of 'nf'=5 formants is used to generate voiced and aspirated sounds, while the parallel branches are used to generate fricatives and plosive bursts.  Since formants are the natural resonant frequencies of the vocal tract, and frequency locations are independent of source location, the formant frequencies of cascade and corresponding parallel resonators must be identical.

	Suggested values for formant frequencies of a number of English sounds were published in Klatt (1980).  """,

('b1', 'b2', 'b3', 'b4', 'b5', 'b6'): """The "formant bandwidth" variables determine the bandwidths of resonators in the cascade vocal tract model.  Since formant bandwidths depend in part on source impedance, and turbulence sources contribute more losses, the synthesizer provides separate control of bandwidths 'p1' 'p2' 'p3' 'p4' 'p5' 'p6' for the parallel formants.

	If the number of formants in the cascade branch is left at the default value of 'nf' = 5, then the 'b6' variable has no meaning and no effect on the synthetic waveform.

	The resonator bandwidth variable has two effects on the frequency-domain shape of the vocal tract transfer function.  An increase in bandwidth reduces the amplitude of the formant peak and simultaneously increases the width of the peak as measured 3 dB down from the peak.  Perceptual experiments indicate that both of these changes have perceptual consequences, but that the change in peak height is much more audible than the width change.

	In a cascade synthesizer, adjustments to formant peak heights in order to match the spectrum of a recorded voice can be achieved either by changing the general slope of the voicing source spectrum (using 'tl') or by changing individual formant bandwidths.  Changing formant bandwidths is an effective way to mimic quite closely the voice quality of a speaker, but some guidelines are offered to help avoid the perceptual problems of aberrant bandwidth specification:

	1. If a bandwidth is set to a value less than the soft limits implemented here, there is a danger that whistle-like harmonics will be heard when a harmonic of the fundamental sweeps past the formant frequency.

	2.  If the bandwidths of the lower formants are wider than the maximum values here, the synthetic voice will begin to sound buzzy.  In this case, all bandwidths should be reduced, and then 'av' can be reduced to get back to an appropriate overall spectral level.""",

('fz', 'fp'):	"""The variable 'fp', "frequency nasal pole", in consort with the variable 'fz', "frequency nasal zero", can mimic the primary spectral effects of nasalization in vowel-like spectra.  In a typical nasalized vowel, the first formant is split into peak-valley-peak (pole-zero-pole) such that 'fp' is at about 300 Hz, 'F1' is higher than it would be if the vowel were non-nasalized, and 'fz' is at a frequency approximately halfway between 'fp' and 'F1'.

	When returning to a non-nasalized vowel, 'fz' is moved down gradually to a frequency exactly the same as 'fp'.  The nasal pole and nasal zero then cancel each other out, and it is as if they were not present in the cascade vocal tract model.""",

('bz', 'bp'):	"""The variables 'bp', "bandwidth nasal pole", and 'bz', "bandwidth nasal zero", are set to default values of 90 Hz.  It is difficult to determine appropriate synthesis bandwidths for individual nasalized vowels, but, fortunately, one can achieve good synthesis results without changing these default values in most cases.""",

'ah':	"""The variable 'ah', "amplitude of aspiration", is the amplitude in dB of the aspiration noise sound source that is combined with periodic voicing, if present ('av'>0), to constitute the glottal sound source that is sent to the cascade vocal tract.  (Voicing can be sent to the parallel vocal tract by making 'ap' non-zero, but aspiration cannot be sent to the parallel vocal tract.  Instead, one would use 'af', the amplitude of frication noise.)  A value of zero turns off the aspiration source, while a value of 60 results in an output aspirated speech sound with levels in formants above F1 roughly equal to the levels obtained by setting 'av' to 60.

	The spectrum of the aspiration noise source is nearly flat, actually falling slightly with increasing frequency.  To best approximate an aspirated speech sound, one should probably increase 'b1', the first formant bandwidth, to anywhere from 200 to 400 Hz, thus simulating the effect of additional low-frequency losses incurred when the glottis is partially open.""",

'oq':	"""The spectrum of a voicing source pulse train can vary in two fairly distinct ways.  The relative amplitude of the first harmonic can increase or decrease, or the general tilt of the spectrum can go up and down.  To change primarily just the first harmonic amplitude, the 'oq' parameter is varied, while the parameter 'tl' affects the general spectral tilt.

	The variable 'oq', "percent of voicing period with glottis open", is a nominal indicator of the width of the glottal pulse when using the default impulse train glottal source, and it is the exact number of samples in the open period when using the natural voicing source ('ss'=2).  A value of 'oq'=50, the default value, corresponds to a 5 msec open portion of the fundamental period at the default sampling rate(10000 samples/sec) and default F0(100 Hz).

	There are many male speakers for whom the duration of the open portion of the fundamental period does not change as fundamental frequency changes over a fairly wide range.  To simulate the behavior of this kind of speaker, one must adjust 'oq' to be inversely proportional to the fundamendal frequency parameter 'f0', which is rather a bother.

	Other speakers tend to produce speech with the duration of the open portion of the cycle being a constant fraction of the total period, e.g. about half of the period.  To synthesize this type of speech it is not necessary to change 'oq' during synthesis.

	The effect of changes in 'oq' on the spectrum is illustrated in Figure 3.  A narrow glottal pulse, as may occur in creaky voice, or when trying to speak loudly, results in a spectrum relatively rich in higher-frequency components, while a wider glottal pulse, as may occur in a breathy offset to speaking, results in a spectrum rich in energy below the first formant.  Thus to match an observed strong first harmonic in the spectrum of a natural utterance, increase 'oq'.

	The synthesizer routine checks to see that 'oq' does not result in an open portion of the glottal pulse which exceeds the duration of the period, and truncates requests that exceed the duration of the current period and prints a warning to the user about the inappropriate value of 'oq'.""",

'at':	"""The variable 'at', "amplitude of turbulence", is the amplitude in dB of turbulence noise generated at the glottis during the open phase of a glottal vibration.  The noise is identical to aspiration except (1) the source is turned off during the closed phase of a glottal cycle, and (2) the output level rises and falls with changes to the variable 'av'.  Thus this breathiness dimension of voicing is zero when 'av' is set to zero, whereas aspiration noise is not influenced by the setting of 'av'.  Usually 'ah' is used to generate aspiration for voiceless aspirated plosives and [h] sounds, while 'at' is used to add a breathiness quality to the voicing source.

	A value of 60 will make the voice quite breathy.  To achieve a good match to natural breathiness, however, one should probably also tilt down the source spectrum, using 'tl', increase the open phase of a glottal cycle, 'oq', to a little more than half the period, and perhaps increase 'b1'.""",

'tl':	"""The variable 'tl', "spectral tilt of voicing", is the (additional) downward tilt of the spectrum of the voicing source, in dB as realized by a soft one-pole low-pass filter.  The effect of changes in 'tl' on the voicing source spectrum is illustrated in Figure 4.  A value of zero has no effect on the source spectrum, while a value of 24 tilts the spectrum down gradually such that frequency components above about 3 kHz are attenuated by about 24 dB relative to a more normal source spectrum.

	The tilt parameter is an attempt to simulate the spectral effect of a "rounding of the corner" at the time of closure in the glottal volume velocity waveform due either to an incomplete closure, as in breathiness, or an asynchronous closure such that the anterior portion of the vocal folds meet at the midline before the posterior portions come together.

	The tilt parameter is also useful in simulating a voicebar, wherein only lower-frequency components are radiated from the closed vocal tract.  For many speech synthesis situations, this would be the only use for 'tl'.  However, 'tl' is a good parameter to use in attempts at matching the spectral details of a particular natural utterance.""",

'af':	"""The variable 'af', "amplitude frication", determines the level of frication noise sent to the various parallel formants and bypass path. The variable should be turned on gradually for fricatives (e.g. straight line from 0 to 60 dB in 90 msec), and abruptly to about 60 dB for plosive bursts.""",

'sk':	"""The variable 'sk', "skew to alternate periods", is the number of 25 microsecond increments to be added to and subtracted from successive fundamental period durations in order to simulate one aspect of vocal fry, the tendency for alternate periods to be more similar in duration than adjacent periods.

	Such aperiodicities, when introduced, have fairly strong perceptual consequences.  This kind of change to normal voicing occurs throughout speech for some voices, and at the initiation and cessation of voicing in a sentence for many others.  There is no need to play with this parameter in most synthesis situations.""",

('a1', 'a2', 'a3', 'a4', 'a5', 'a6', 'ab'):	"""The variables 'a1' 'a2' 'a3' 'a4' 'a5' 'a6' 'ab', "amplitudes parallel formants", determine the spectral shape of a fricative or plosive burst.  If a formant is a front cavity resonance for a particular fricative articulation, one might set the formant amplitude to 60 dB as a first guess.  Formants associated with the cavity in back of the constriction should have their amplitudes set to zero initially, (The amplitude of the first parallel formant,'a1', is therefore zero for all English fricatives.) and then all parallel formant amplitudes should be adjusted on a trial-and-error basis, comparing synthesized frication spectra with a natural frication spectrum.

	The bypass path amplitude ('ab') is used when the vocal tract resonance effects are negligible because the cavity in front of the main fricative constriction is too short, as in [f], [v], [th], [dh], [p], [b].""",

('p1', 'p2', 'p3', 'p4', 'p5', 'p6'): """The variables 'p1' 'p2' 'p3' p4' 'p5' 'p6', "bandwidths parallel formants" are set to default values that are wider than the bandwidths used in the cascade vocal tract model.  It is difficult to measure formant bandwidths accurately in noise spectra, even when a fairly long sustained fricative is available for analysis.  However, these default values can be used in most situations.  The only adjustment is then made to the parallel formant amplitudes in order to match details in a natural frication spectrum.""",

('ap', 'an'):	"""All-Parallel Synthesis Using 'ap' and 'an'. The variable 'ap', "amplitude voicing parallel", is the amplitude, in dB, of voiced excitation of the parallel vocal tract.  Normally, this would be allowed to remain at the default value of zero since the cascade vocal tract would be used for generating the voicing component of all voiced sounds (even voicebars and voiced fricatives).

    However, there are circumstances where a vowel with special characteristics (e.g. two-formant vowels) can only be generated using the greater flexibility (individual control of formant amplitudes) of the parallel vocal tract.  A value of 'ap' = 60 would be a good choice to synthesize a typical vowel using the parallel vocal tract model. Of course, 'av', would be set to zero.

    The parallel formant amplitude variables must then be adjusted to get the right spectral shape for the vowel.  A good starting point is to set parallel formant amplitudes 'a1' 'a2' 'a3' 'a4' 'a5' to 60 dB. This will give exactly the right relative formant amplitudes for a non-nasalized vowel with formant frequencies at 500, 1500, 2500, 3500 and 4500 Hz.  However, as formant frequencies are changed from these values (appropriate for a uniform tube), formant amplitudes can quickly diverge from those in a corresponding cascade vocal tract model. (The formant amplitude will increase/decrease as formant frequency is increased/decreased, but there is no automatic adjustment such that formants "riding on the skirt" of a lower-frequency formant are attenuated as this formant frequency is lowered.)  Trial-and-error adjustment of parallel formant amplitudes is then necessary.

    The variable 'an', "amplitude parallel nasal formant", is normally not used.  However, when employing the parallel vocal tract to synthesize vowels, as discussed above, 'an' can be used to simulate the effects of nasalization on vowels and nasal murmurs.  To achieve nasalization, one would set 'fp' to about 280 Hz (the default value) and adjust both 'an' and 'a1' to levels matching a nasalized vowel spectrum.""",

'os':	"""The constant 'os', "output waveform selector", determines which waveform is saved in the output file.  If 'os' has the default value of zero, the normal final output of synthesis is saved. Other output options are given in Table III.  For example, if you wished to see and spectrally analyze the voicing source waveform of the synthesizer by itself for a particular synthetic utterance, you would set 'os' to four.

	Note that the radiation characteristic is applied if 'os' is greater than 4, but not if 'os' is less than 4. (Due to computational considerations, the derivative of the voicing source is usually computed directly, so that the actualsource waveform that is displayed when requested is approximated by sending the computed source waveform through a leaky integrator.)  Thus, setting'os' to 4 results in the actual voicing source waveform being generated, while setting 'os' to 5 produces the (first difference) of the voicing source waveform that ordinarily is routed to the parallel vocal tract model.

	KLSYN output waveform options using 'os'.   

os	             WAVEFORM SAVED 		
0.    	Normal synthesis output    
1.    	Voicing periodic component alone    
2.    	Aspiration alone    
3.    	Frication alone    
4.    	Glottal source (voicing, turbulence, and aspiration)    
5.    	Glottal source sent to parallel vocal tract (AP) + radiation char    
6.    	Cascade vocal tract, output of nasal zero resonator    "  
7.    	Cascade vocal tract, output of nasal pole resonator    "    
8.    	Cascade vocal tract, output of fifth formant           "    
9.    	Cascade vocal tract, output of fourth formant          "   
10.   	Cascade vocal tract, output of third formant           "   
11.    	Cascade vocal tract, output of second formant       "   
12.    	Cascade vocal tract, output of first formant            "   
13.    	Parallel vocal tract, output of sixth formant alone     "   
14.    	Parallel vocal tract, output of fifth formant alone     "   
15.    	Parallel vocal tract, output of fourth formant alone    "  
16.    	Parallel vocal tract, output of third formant alone     "   
17.    	Parallel vocal tract, output of second formant alone    "   
18.    	Parallel vocal tract, output of first formant alone     "   
19.    	Parallel vocal tract, output of nasal formant alone     "   
20.   	 Parallel vocal tract, output of bypass path alone      "       """,

'g0':	"""An overall gain control, 'g0', is included to permit the user to adjust the output level without having to modify each source amplitude time function.  The nominal value is 60 dB.  To increase the output by e.g. 3 dB, one would simply set 'g0' to 63.""",

'dF' 'db':	"""The dF and db parameters are obscure and rarely used.  They control a degree of flutter in the value of F1 and b1 as a function of the glottal state to perhaps improve the naturalness of the voice.""",
   }
       
def ifc2klp(fname,starttime,endtime):
    ''' read an ifcformant formant traces file and convert to params for the klatt synthesizer '''
         
    sep = "\s+"
    fields = None
    field_map = {}
    params = {}
    nonklatt_params={}
    comments = {'header': '#  produced by ifc2klp\n', 'constant': {}, 'varied': []}
    print('start {} to end {}'.format(starttime,endtime))

    with open(fname, 'r') as f:
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
                            if val<starttime:
                                break
                            if endtime != -1 and val > endtime:
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
                        
            params['ui'] = nonklatt_params['sec'][1]-nonklatt_params['sec'][0]
            params['du'] = nonklatt_params['sec'][len(nonklatt_params['sec'])-1]+params['ui']
            params['nf'] = 4

    params['av'] = []
    params['af'] = []
    rms_change = 60 - max(nonklatt_params['rms'])
    for i in range(0,len(nonklatt_params['rms'])):
        scaledrms = max(0,nonklatt_params['rms'][i]+rms_change)
        if params['f0'][i]>0:
            params['av'].append(scaledrms)
            params['af'].append(0)
        else:
            params['av'].append(0)
            params['af'].append(scaledrms)
    # now fill in all params with default values using synthesizer
    synth = kw.synthesizer()
    synth.set_params(params)
    for (param, val) in synth.get_constant_params().items():
            params[param]=val
    return params,comments
    
def interpolate(params,time,val,par): 
    ui = params['ui']
    t = int(ui*round(float(time)/ui))  # force it to multiple of ui
    v = float(val)
        
    if interpolate.ot == None:   # first time in function
        interpolate.ot = t
        interpolate.ov = v
        return
    if t==interpolate.ot:
        idx = int(t/ui)
        params[par].pop(idx)
        params[par].insert(idx,val)
        interpolate.ov=v
        return

    du = params['du']
    tvalues = np.arange(0,du,ui)
    yvalues = params[par]
    if not isinstance(yvalues, list):
            yvalues = [yvalues]*len(tvalues)
    params[par]=yvalues
   
    if t>interpolate.ot:
        dt = t - interpolate.ot
        dv = v - interpolate.ov
        for t1 in range(interpolate.ot,t+ui,ui):
            idx = int(t1/ui)
            newv = int(round(interpolate.ov + ((dv*(t1-interpolate.ot)/dt))))
            params[par].pop(idx)                  
            params[par].insert(idx,newv)
    else:
        dt = interpolate.ot - t
        dv = interpolate.ov - v
        for t1 in range(interpolate.ot,t,-ui):
            idx = int(t1/ui)
            newv = int(round(v + ((dv*(t1-t)/dt))))
            params[par].pop(idx)                  
            params[par].insert(idx,newv)
    interpolate.ov = v
    interpolate.ot = t
interpolate.ot = None
interpolate.ov = None
    
class DataCursor(object):
    text_template = 'x: %0.2f\ny: %0.2f'
    x, y = 0.0, 0.0
    xoffset, yoffset = -20, 20
    text_template = 'x: %0.2f\ny: %0.2f'

    def __init__(self, ax):
        self.ax = ax
        self.annotation = ax.annotate(self.text_template, 
                xy=(self.x, self.y), xytext=(self.xoffset, self.yoffset), 
                textcoords='offset points', ha='right', va='bottom',
                bbox=dict(boxstyle='round,pad=0.5', fc='yellow', alpha=0.5),
                arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0')
                )
        self.annotation.set_visible(False)

    def __call__(self, event):
        self.event = event
        # xdata, ydata = event.artist.get_data()
        # self.x, self.y = xdata[event.ind], ydata[event.ind]
        self.x, self.y = event.xdata, event.ydata
        print('in DataCursor(), x={} and y={}'.format(self.x,self.y))
        if self.x is not None:
            print('plot it')
            self.annotation.xy = self.x, self.y
            self.annotation.set_text(self.text_template % (self.x, self.y))
            self.annotation.set_visible(True)
            event.canvas.draw()
    
    
class EnterPanel(wx.Panel):
    def __init__(self, parent, **kws):
        wx.Panel.__init__(self,parent, -1, **kws)
        (wW,wH) = parent.GetSize()
        self.params = parent.params
        self.par = None
        dpi = 80
        wW = wW/dpi - 0.5
        wH = wH/dpi *.30
        self.figure = matplotlib.figure.Figure(figsize=(wW,wH),facecolor="w")
        self.canvas = FigureCanvas(self,-1,self.figure)
        self.axes = self.figure.add_subplot(111)
#        self.canvas.callbacks.connect('motion_notify_event', self.mouseMotion)
        self.canvas.callbacks.connect('button_release_event', self.mouseUp)   
        self.canvas.mpl_connect('motion_notify_event', DataCursor(plt.gca()))         
            
    def mouseMotion(self, evt):
        x, y = evt.xdata, evt.ydata
        if x is None:  # outside the axes
             return           
    
    def mouseUp(self,evt):
        x, y = evt.xdata, evt.ydata
        ui = self.params['ui']        
        if x is None:  # outside the axes
             return           
        x = round(x/ui)*ui
        y = int(round(y))
        print('par={}, x={}, y={}'.format(self.par,x,y))
        interpolate(self.params,x,y,self.par) 
        self.line.set_ydata(self.params[self.par])
        self.canvas.draw()
    
    def draw(self,parent,par):
        self.par = par
        self.params = parent.params
        self.axes.clear()
        if par == None:
            self.par = 'f0'
        ui = self.params['ui']
        du = self.params['du']
        t = np.arange(0,du,ui)
        v = self.params[self.par]
        if not isinstance(v, list):
            v = [v]*len(t)
        self.line, = self.axes.plot(t,v,color="black")
        self.startline, = self.axes.plot(t,v,color="lightgray")
        self.axes.set_title("Now drawing '{}': {}".format(par,param_info[self.par]['name']))
        min=param_info[self.par]['min']
        max=param_info[self.par]['max']
        self.axes.set_ybound(min,max)
        self.canvas.draw()

        interpolate.ot = None
        interpolate.ov = None

class myPlotPanel(wx.Panel):
    def __init__(self, parent, **kws):
        wx.Panel.__init__(self,parent, -1, **kws)
        (wW,wH) = parent.GetSize()
        dpi = 80
        wW = wW/dpi-0.5
        wH = wH/dpi *.60
        self.figure = matplotlib.figure.Figure(figsize=(wW,wH),facecolor="w")
        self.canvas = FigureCanvas(self,-1,self.figure)
        self.freqax = self.figure.add_subplot(111)
        self.ampax = self.freqax.twinx()

    
    def draw(self,parent):
        self.freqax.clear()
        self.ampax.clear()

        params = parent.params  
        self.freqax.set_ylim((0,params['sr']/2))
        self.freqax.set_xlabel( "Time (ms)" )
        self.freqax.set_ylabel( "Frequency (Hz)" )
 
        self.ampax.set_ylim((0,70))
        self.ampax.set_ylabel( "Amplitude (dB)")
        
        ui = params['ui']
        du = params['du']
        t = np.arange(0,du,ui)

        f1 = params['F1']
        if not isinstance(f1, list):
            f1 = [f1]*len(t)
        f2 = params['F2']
        if not isinstance(f2, list):
            f2 = [f2]*len(t)
        f3 = params['F3']
        if not isinstance(f3, list):
            f3 = [f3]*len(t)
        f4 = params['F4']
        if not isinstance(f4, list):
            f4 = [f4]*len(t)
        f5 = params['F5']
        if not isinstance(f5, list):
            f5 = [f5]*len(t)
        f6 = params['f6']
        if not isinstance(f6, list):
            f6 = [f6]*len(t)
 

        b1 = params['b1']
        if not isinstance(b1, list):
            b1 = [b1]*len(t)
        b2 = params['b2']
        if not isinstance(b2, list):
            b2 = [b2]*len(t)
        b3 = params['b3']
        if not isinstance(b3, list):
            b3 = [b3]*len(t)

        av = params['av']
        if not isinstance(av, list):
            av = [av]*len(t)
        af = params['af']
        if not isinstance(af, list):
            af = [af]*len(t)
        scale = [x/60.0 for x in av]
        b1 = np.array(b1)*np.array(scale)
        b2 = np.array(b2)*np.array(scale)
        b3 = np.array(b3)*np.array(scale)

        self.ampax.plot(t,av,color="red")
        self.ampax.plot(t,af,color="orange")

        self.freqax.plot(t,f1,color="blue",label="F1")
        self.freqax.plot(t,f2,color="blue")
        self.freqax.plot(t,f3,color="blue")
#        self.axes.axvline(x=100,color="black")
        self.canvas.draw()

    
class infoDialog(wx.Dialog):
    def __init__(self, parent,pos):
        wx.Dialog.__init__(self, parent, title="Disclaimer",pos=pos)       
   
        par=parent.par
        min = param_info[par]['min']
        max = param_info[par]['max']
        name = param_info[par]['name']
        cap_text = '{} - {} (min={},max={})'.format(par,name,min,max)
        caption = wx.TextCtrl(self,-1,cap_text,style=wx.TE_READONLY)
        
        for k in param_text.keys():
            if par in k:
                break
        txt = param_text[k] 
        text = wx.TextCtrl(self, -1, txt, size=(600,400), style=wx.TE_MULTILINE | wx.TE_READONLY)        

        sizer = wx.BoxSizer(wx.VERTICAL )        
        btnsizer = wx.BoxSizer()
        btn = wx.Button(self, wx.ID_OK)
        btnsizer.Add(btn, 0, wx.ALL, 5)
        btnsizer.Add((5,-1), 0, wx.ALL, 5)
        sizer.Add(caption, 0, wx.EXPAND|wx.ALL, 5)
        sizer.Add(text, 0, wx.EXPAND|wx.ALL, 5)    
        sizer.Add(btnsizer, 0, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)    
        self.SetSizerAndFit (sizer)

ID_PLAY = wx.NewId()

def playaudio(filename):
    if 'Windows' in platform.system():
        p = subprocess.Popen(["sox", "-q", filename, "-d"],shell=True)
    else:
        p = subprocess.Popen(["sox", "-q", filename, "-d"],shell=False)
    p.wait()  
    

class MainFrame(wx.Frame):
    def __init__(self, parent,*args, **kwargs):
        wx.Frame.__init__(self, parent, *args, **kwargs)
        self.SetBackgroundColour("white")
        
        self.params = {'du':500,'ui':5}
        self.par = 'f0'
        self.comments = {'header': '#  produced by wxpython\n', 'constant': {}, 'varied': []}
        synth = kw.synthesizer()
        synth.set_params(self.params)
        for (param, val) in synth.get_constant_params().items():
            self.params[param]=val
        self.fname = "klatt_temp"

        self.Bind(wx.EVT_CLOSE, self.OnClose)

        # menu structure
        menuBar = wx.MenuBar()
        exitMenu = wx.Menu()
        m_exit = exitMenu.Append(wx.ID_EXIT, "E&xit\tAlt-X", "Close window and exit program.")
        self.Bind(wx.EVT_MENU, self.OnClose, m_exit)
        fileMenu = wx.Menu()
        m_new = fileMenu.Append(wx.ID_NEW,"N&ew\tCtrl-N","Start a new synthesis file.")
        m_open = fileMenu.Append(wx.ID_OPEN,"O&pen\tCtrl-O","Open a synthesis parameter file.")
        m_save = fileMenu.Append(wx.ID_SAVE,"S&ave\tCtrl-S","Save .wav and .klp files.")
        m_play = fileMenu.Append(ID_PLAY,"P&lay\tCtrl-P","Play synthesized speech.")
        m_quit = fileMenu.Append(wx.ID_EXIT, 'Quit', 'Quit')
        menuBar.Append(fileMenu, '&File')
        menu=wx.Menu()       
        m_about = menu.Append(wx.ID_ABOUT, "&About", "Information about this program")
        menuBar.Append(menu, "&Help")

        self.Bind(wx.EVT_MENU, self.OnSave, m_save)
        self.Bind(wx.EVT_MENU, self.OnNew, m_new)
        self.Bind(wx.EVT_MENU, self.OnClose, m_quit)
        self.Bind(wx.EVT_MENU, self.OnOpen, m_open)
        self.Bind(wx.EVT_MENU, self.OnPlay, m_play)
        self.Bind(wx.EVT_MENU, self.OnAbout, m_about)   

        # the logical organization of the screen - bars, fields and buttons
        self.SetMenuBar(menuBar)
        self.statusbar = self.CreateStatusBar()
        
        self.m_text = wx.StaticText(self, wx.ID_ANY, label="Klsyn")
        self.m_text.SetFont(wx.Font(28, wx.ROMAN,wx.NORMAL, wx.BOLD))
        self.m_text.SetSize(self.m_text.GetBestSize())
        self.b_text = wx.StaticText(self, wx.ID_ANY, label="")
        #self.b_text.SetSize(self.b_text.GetBestSize())
    
        self.panel = myPlotPanel(self)
        self.panel.draw(self)
        self.epanel = EnterPanel(self)
        self.epanel.draw(self,par="f0")

        

        newbutton = wx.Button(self, id= wx.ID_ANY, label="New")
        newbutton.Bind(wx.EVT_BUTTON, self.OnNew)
        openbutton = wx.Button(self, id= wx.ID_ANY, label="Open")
        openbutton.Bind(wx.EVT_BUTTON, self.OnOpen)
        savebutton = wx.Button(self, id= wx.ID_ANY, label="Save")
        savebutton.Bind(wx.EVT_BUTTON, self.OnSave)
        playbutton = wx.Button(self, id= wx.ID_ANY, label="Play")
        playbutton.Bind(wx.EVT_BUTTON, self.OnPlay)
        
        instruct = wx.StaticText(self, wx.ID_ANY, label="Draw:")

        constantparams= ['du','ui','nf','ss','os','rs','g0','db','dF','agc']
        const_text = wx.StaticText(self, wx.ID_ANY, label="Constants")
        const = wx.Choice(self,choices = constantparams,name="const")
        const.Bind(wx.EVT_CHOICE,self.ChangeConst)

        voiceparams = ['av','f0','at','tl','sk','oq']
        voice_text = wx.StaticText(self, wx.ID_ANY, label="Voice")
        voice = wx.Choice(self,choices = voiceparams,name="voice")
        voice.Bind(wx.EVT_CHOICE,self.DrawParam)

        cascadeparams = ['F1','F2','F3','F4','F5','f6','b1','b2','b3','b4','b5',
                         'b6','fp','fz','bp','bz']
        cascade_text = wx.StaticText(self, wx.ID_ANY, label="Cascade")
        cascade = wx.Choice(self,choices = cascadeparams,name="cascade")
        cascade.Bind(wx.EVT_CHOICE,self.DrawParam)
        
        parallelparams = ['af','ah','a1','a2','a3','a4','a5','a6','p1','p2','p3',
                          'p4','p5','p6','ab','ap','an']
        parallel_text = wx.StaticText(self, wx.ID_ANY, label="Parallel")
        parallel = wx.Choice(self,choices = parallelparams,name="parallel",style=wx.CB_SORT)
        parallel.Bind(wx.EVT_CHOICE,self.DrawParam)
        
        infobutton = wx.Button(self, id= wx.ID_ANY, label="Info")
        infobutton.Bind(wx.EVT_BUTTON, self.InfoBox)
               
        leftcolumn = wx.BoxSizer(wx.VERTICAL)
        leftcolumn.Add(self.m_text, flag=wx.ALIGN_CENTER)
        leftcolumn.AddSpacer(10)
        leftcolumn.Add(newbutton, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(5)
        leftcolumn.Add(openbutton, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(5)
        leftcolumn.Add(savebutton, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(5)
        leftcolumn.Add(playbutton, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(60)
        leftcolumn.Add(const_text,5,flag=wx.ALIGN_RIGHT)
        leftcolumn.Add(const, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(30)
        leftcolumn.Add(instruct,flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(10)
        leftcolumn.Add(voice_text,5, flag=wx.ALIGN_RIGHT)
        leftcolumn.Add(voice, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(10)
        leftcolumn.Add(cascade_text,5, flag=wx.ALIGN_RIGHT)
        leftcolumn.Add(cascade, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(10)
        leftcolumn.Add(parallel_text,5,flag=wx.ALIGN_RIGHT)
        leftcolumn.Add(parallel, flag=wx.ALIGN_RIGHT)
        leftcolumn.AddSpacer(30)
        leftcolumn.Add(infobutton, flag=wx.ALIGN_RIGHT)


        self.rightcolumn=wx.BoxSizer(wx.VERTICAL)
        self.rightcolumn.Add(self.panel,flag=wx.EXPAND)
        self.rightcolumn.Add(self.epanel,flag=wx.EXPAND)
        self.rightcolumn.AddSpacer(10)
        self.rightcolumn.Add(self.b_text,flag=wx.ALIGN_LEFT)
        
        self.box = wx.BoxSizer(wx.HORIZONTAL)
        self.box.Add(leftcolumn)
        self.box.AddSpacer(10)
        self.box.Add(self.rightcolumn,flag=wx.ALIGN_TOP)

        self.SetAutoLayout(True)
        self.SetSizer(self.box)
        self.Layout()
    
            
    def InfoBox(self,parent):
        message= infoDialog(self,pos=(200,200)) 
        message.ShowModal()
        message.Destroy()

    def ChangeConst(self,event):
        self.par = event.GetString()
        name = param_info[self.par]['name']
        max = param_info[self.par]['max']
        min = param_info[self.par]['min']
        entry = wx.TextEntryDialog(self, '{} - {} (min={},max={}'.format(self.par,name,min,max), 'Enter Value')
        entry.ShowModal()
        val = entry.GetValue()
        if val==None:   # make no change if no value is entered
            return
        self.params[self.par]=int(val)
        
    def DrawParam(self,event):
        self.par = event.GetString()
        self.epanel.draw(self,self.par)
            
    def OnNew(self,e):      
        self.comments = {'header': '#  produced by wxpython\n', 'constant': {}, 'varied': []}
        self.params = {'du':500,'ui':5}
        synth = kw.synthesizer()
        synth.set_params(self.params)
        for (param, val) in synth.get_constant_params().items():
            self.params[param]=val
        self.fname = "klatt_temp"
        self.statusbar.SetStatusText('Reset Synthesis parameters')
        self.panel.draw(self)
        self.epanel.draw(self,par=None)


    def AskQuestion(self,e,quest):
        dlg = wx.TextEntryDialog(None, quest,"","", style=wx.OK)
        dlg.ShowModal()
        dlg.Destroy()
        return dlg.GetValue()

    def OnSave(self,e):
        temp = self.AskQuestion(self,"First name of .wav and .klp files?")
        try:
            self.fname, fext = os.path.splitext(temp)
        except ValueError:
            self.fname = temp
        synth = kw.synthesizer()
        synth.set_params(self.params)
        (d,rate) = synth.synthesize()
        scipy.io.wavfile.write(self.fname + '.wav', rate, d)
        klsyn.klpfile.write(self.fname + '.klp', synth=synth, comments=self.comments)
        self.statusbar.SetStatusText("\tFiles {} and {} were saved.".format(self.fname+'.wav',self.fname+'.klp'))

    def OnOpen(self,e):
        print("in OnOpen .... ")
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a file", dirname, "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            os.chdir(dirname)
            
            self.fname, fext = os.path.splitext(filename)
        
            try:
                f = open(filename, 'r')
            except:
                wx.MessageBox('Didn\'t find file: {}'.format(filename), 'Klsyn', wx.OK | wx.ICON_EXCLAMATION)
                return
            
            d=f.read()
            f.close()
        
            if re.search('\s*_varied_params_\s*', d, re.MULTILINE) != None:  # looks like a .klp file
                (self.params, self.comments) = klsyn.klpfile.read(filename)

            elif (re.match('^sec*.',d) != None):   # looks like an ifcformant file
                start = 0.0
                end = -1.0
                start = float(self.AskQuestion(self,"Reading ifcformants file - start time?"))
                end = float(self.AskQuestion(self,"Reading ifcformants file - end time?"))
                if end != -1 and end<=start:
                    wx.MessageBox('End time {} less than start time {}, not allowed.'.format(end,start), 'Klsyn', wx.OK | wx.ICON_EXCLAMATION)
                    return
                (self.params,self.comments) = ifc2klp(filename,start,end)
            else:
                wx.MessageBox('the file format of {} was not recognized.'.format(filename), 'Klsyn', wx.OK | wx.ICON_EXCLAMATION)
                return
            self.statusbar.SetStatusText("Finished reading the file {}".format(filename))
            self.panel.draw(self)
            self.epanel.draw(self,par='f0')

    def OnPlay(self,e):
        synth = kw.synthesizer()
        synth.set_params(self.params)
        (d,rate) = synth.synthesize()
        scipy.io.wavfile.write('klattsyn_temp.wav', rate, d)
        playaudio('klattsyn_temp.wav')
        self.statusbar.SetStatusText("Playing the sound")
        self.panel.draw(self)
        self.epanel.draw(self,par=None)
        
    def OnClose(self, event):
        self.Destroy()
        sys.exit()
        

    def OnAbout(self, event):
        dlg = wx.MessageDialog(self,
            "A graphic interface for Dennis Klatt's synthesizer.\n by Keith Johnson",
            "Klsyn:", wx.OK)
        dlg.ShowModal()
        dlg.Destroy()

#-------------------------------------------------------------------

class MyApp(wx.App):

    def OnInit(self):
        dw, dh = wx.DisplaySize()
        w = int(dw*0.7)
        h = int(dh*0.7)
        frame = MainFrame(None,-1,"Klsyn",size=(w,h))
        frame.SetPosition((30, 30))

        frame.Show(True)


        self.SetTopWindow(frame)
        return True

#-------------------------------------------------------------------

def main():
    print('main is running...')
    app = MyApp(0)
    app.MainLoop()

#-------------------------------------------------------------------

if __name__ == "__main__" :
    main()

