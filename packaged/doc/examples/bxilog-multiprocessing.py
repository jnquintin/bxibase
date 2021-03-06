#!/usr/bin/env python
import bxi.base.log as bxilog
import multiprocessing
import tempfile
import os
import sys


# [BXI Log and Python multiprocessing module]
@bxilog.multiprocessing_target
def my_function(logfilename):
    # Since a fork() has been made, the logging module must be initialized
    bxilog.basicConfig(filename=logfilename, filemode='w')
    # Using the bxilog.multiprocessing_target decorator guarantees that
    # uncaught exception will be reported by the logging system
    # and that the logging system will be cleanup properly (and therefore
    # flushed).
    bxilog.out("In subprocess")
    bxilog.flush()

    try:
        raise ValueError("An expected exception in subprocess")
    except:
        bxilog.exception("Handling an exception in subprocess")

    # This will be catched thanks to the bxilog.multiprocessing_target() decorator
    # Otherwise, the exception will appear on the standard error as usual
    raise ValueError("An unexpected exception in subprocess")
# [BXI Log and Python multiprocessing module]

if __name__ == '__main__':

    filename = tempfile.NamedTemporaryFile(prefix=os.path.basename(sys.argv[0]),
                                           suffix='.bxilog', delete=False).name

    p = multiprocessing.Process(target=my_function, args=[filename])
    p.start()
    p.join()

    with open(filename) as f:
        lines = f.readlines()
        print(lines[-1])
        assert lines[-1].endswith("An unexpected exception in subprocess\n")
