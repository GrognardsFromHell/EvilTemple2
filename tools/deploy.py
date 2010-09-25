
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

distpath = os.path.join(os.getcwd(), 'dist')
print "DistPath:", distpath

#
# This is a slightly modified version of shutil.copytree that ignores existing directories (since
# we merge multiple directories into one)
#
def copyDirectory(src, dst, ignore=None):
    names = os.listdir(src)
    if ignore is not None:
        ignored_names = ignore(src, names)
    else:
        ignored_names = set()

    if not os.path.exists(dst):
        os.makedirs(dst)
    errors = []
    for name in names:
        if name in ignored_names:
            continue
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if os.path.isdir(srcname):
                copyDirectory(srcname, dstname, ignore)
            else:
                shutil.copy2(srcname, dstname)
            # XXX What about devices, sockets etc.?
        except (IOError, os.error), why:
            errors.append((srcname, dstname, str(why)))
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error, err:
            errors.extend(err.args[0])
    try:
        shutil.copystat(src, dst)
    except WindowsError:
        # can't copy file access times on Windows
        pass
    except OSError, why:
        errors.extend((src, dst, str(why)))
    if errors:
        raise shutil.Error(errors)

for (type, source, destination) in dist_files:
    source = os.path.join(basepath, source)
    destination = os.path.join(distpath, destination)
    
    if type == Directory:
        print "Copying directory from", source, "to", destination
        copyDirectory(source, destination)
    else:
        print "File"
