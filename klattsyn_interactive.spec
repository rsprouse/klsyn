# -*- mode: python -*-
a = Analysis(['scripts/klattsyn_interactive.py'],
             pathex=['c:/Python27/Lib/site-packages/', 'c:\\Users\\ronald\\Documents\\GitHub\\klsyn'],
             hiddenimports=[],
             hookspath=None,
             runtime_hooks=None)

# Next line is a hack to prevent annoying warning
# (WARNING: file already exists but should not: C:\Users\<user>\AppData\Local\Temp\_MEI355602\Include\pyconfig.h)
# See http://stackoverflow.com/questions/19055089/pyinstaller-onefile-warning-pyconfig-h-when-importing-scipy-or-scipy-signal
# Inclusion of this hack means we need to build from the .spec file, not the
# script.
a.datas = list({tuple(map(str.upper, t)) for t in a.datas})


pyz = PYZ(a.pure)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name='klattsyn_interactive.exe',
          debug=False,
          strip=None,
          upx=True,
          console=True )
