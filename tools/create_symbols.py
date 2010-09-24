
import os
from zipfile import *

files = [
    'audioengine.pdb',
    'binkplayer.pdb',
    'converter.pdb',
    'eviltemple.pdb',
    'game.pdb',
    'qt3d.pdb',
    'troikaformats.pdb'
]

myzip = ZipFile('symbols.zip', 'w', ZIP_DEFLATED)

for filename in files:
    result = os.popen("dump_syms.exe " + filename).read()
    lines = result.splitlines()
    ident = lines[0].split(" ")
    symbolhash = ident[3]
    myzip.writestr(filename + '/' + symbolhash + '/' + filename.replace('.pdb', '.sym'), result)

myzip.close()
