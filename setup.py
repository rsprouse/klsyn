#!/usr/bin/env python

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import numpy

ext_modules = [
  Extension(
    name="klsyn.klatt_wrap",
    sources=["klsyn/klatt_wrap.pyx"],
    libraries = ["m"],
    include_dirs=[numpy.get_include()],
    language="c",
  )
]

setup(
  name = 'klsyn',
  cmdclass = {'build_ext': build_ext},
  #ext_package='klatt_wrap',
  ext_modules = ext_modules,
  packages = ['klsyn'],
  scripts = [
    'scripts/klattsyn.py',
    'scripts/klattsyn_interactive.py',
    'scripts/klp_continuum.py',
    'scripts/doc2klp.py',
    'scripts/wxklsyn.pyw'
  ],
  classifiers = [
    'Intended Audience :: Science/Research',
    'Topic :: Scientific/Engineering',
    'Topic :: Multimedia :: Sound/Audio :: Speech'
  ]
)
