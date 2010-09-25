
import os
import shutil
import sys

Directory = 1

# All paths are relative to the project's root directory.
dist_files = [
    (Directory, 'data', 'data')
]

def printLine():
    print '--------------------------------------------------------------------------------'

#
# Creates a deployment package
#

printLine()
print 'EvilTemple Deployment Script'
printLine()

# Get the full path of the parent directory
basepath = os.path.normpath(os.path.dirname(os.path.abspath(sys.argv[0])) + '/../')
print "BasePath:", basepath

distpath = os.getcwd()
print "DistPath:", distpath

for (type, source, destination) in dist_files:
    source = os.path.join(basepath, source)
    destination = os.path.join(distpath, destination)
    
    if type == Directory:
        print "Copying directory from", source, "to", destination
        if os.path.exists(destination):
            shutil.rmtree(destination)
        shutil.copytree(source, destination)
    else:
        print "File"
