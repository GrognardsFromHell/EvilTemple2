
import os
import glob
import sys

dist_files = [
    'bin/script'
]

def printLine():
    print '--------------------------------------------------------------------------------'

#
# Creates a deployment package
#

printLine()
print 'EvilTemple Deployment Script'
printLine()

basepath = os.path.normpath(os.path.dirname(os.path.abspath(sys.argv[0])) + '/../')
print "BasePath:", basepath

for distfile in dist_files:
    print(distfile)
