#!/usr/bin/env python
import re
import sys
from pprint import pprint

def enum_options(file):
    # key = *opts*, value = refid, remove proceeding -- from long
    # option name and s/-/_/g and prepend '_optref'
    ref_db = {}
    p = re.compile(r'^\*(-[a-zA-Z0-9-]+)\*(?:,\s*\*(-[a-zA-Z0-9-]+)\*)?.*::$')
    for line in file:
        m = p.match(line)
        if m:
            if m.group(2) is None:
                short_opt = None
                long_opt = m.group(1)
            else:
                short_opt = m.group(1)
                long_opt = m.group(2)
            ref_id = make_ref_id(long_opt)

            if short_opt in ref_db:
                print "warn: duplicate short_opt", short_opt
            if long_opt in ref_db:
                print "warn: duplicate long_opt", log_opt

            if short_opt:
                ref_db[short_opt] = ref_id
            ref_db[long_opt] = ref_id
    return ref_db

def make_ref_id(long_opt):
    return 'aria2_optref_'+long_opt.strip('*').lstrip('-').replace('-', '_')

def make_cross_ref(out, file, ref_db):
    opt_def = re.compile(r'^\*(-[a-zA-Z0-9-]+)\*(?:,\s*\*(-[a-zA-Z0-9-]+)\*)?.*::$')
    opt_ref = re.compile(r'\*(-[a-zA-Z0-9-]+)\*')
    for line in file:
        m = opt_def.match(line)
        if m:
            if m.group(2) is None:
                long_opt = m.group(1)
            else:
                long_opt = m.group(2)
            out.write('[[{0}]]'.format(ref_db[long_opt]))
            out.write(line)
            continue
        pos = 0
        while 1:
            m = opt_ref.search(line, pos)
            if m:
                opt = line[m.start(1):m.end(1)]
                if opt in ref_db:
                    out.write(line[pos:m.start(0)])
                    out.write('*<<{0}, {1}>>*'.format(ref_db[opt], opt))
                else:
                    print "warn: not in ref_db", opt
                    out.write(line[pos:m.end(0)])
                pos = m.end(0)
            else:
                out.write(line[pos:])
                break

if __name__ == '__main__':
    with open(sys.argv[1]) as f:
        ref_db = enum_options(f)
    with open(sys.argv[1]) as f:
        with open(sys.argv[2], 'wb') as out:
            make_cross_ref(out, f, ref_db)

