#!/usr/bin/env python
#
# aria2 - The high speed download utility
#
# Copyright (C) 2013 Tatsuhiro Tsujikawa
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# In addition, as a special exception, the copyright holders give
# permission to link the code of portions of this program with the
# OpenSSL library under certain conditions as described in each
# individual source file, and distribute linked combinations
# including the two.
# You must obey the GNU General Public License in all respects
# for all of the code used other than OpenSSL.  If you modify
# file(s) with this exception, you may extend this exception to your
# version of the file(s), but you are not obligated to do so.  If you
# do not wish to do so, delete this exception statement from your
# version.  If you delete this exception statement from all source
# files in the program, then also delete it here.
#
# Generates API reference from C++ source code.
from __future__ import print_function
import re, sys, argparse

class FunctionDoc:
    def __init__(self, name, content, domain):
        self.name = name
        self.content = content
        self.domain = domain

    def write(self, out):
        print('''.. {}:: {}'''.format(self.domain, self.name))
        print()
        for line in self.content:
            print('    {}'.format(line))

class TypedefDoc:
    def __init__(self, name, content):
        self.name = name
        self.content = content

    def write(self, out):
        print('''.. type:: {}'''.format(self.name))
        print()
        for line in self.content:
            print('    {}'.format(line))

class StructDoc:
    def __init__(self, name, content, domain, members, member_domain):
        self.name = name
        self.content = content
        self.domain = domain
        self.members = members
        self.member_domain = member_domain

    def write(self, out):
        if self.name:
            print('''.. {}:: {}'''.format(self.domain, self.name))
            print()
            for line in self.content:
                print('    {}'.format(line))
            print()
            for name, content in self.members:
                name = name.strip()
                # For function (e.g., int foo())
                m = re.match(r'(.+)\s+([^ ]+\(.*)', name)
                if not m:
                    # For variable (e.g., bool a)
                    m = re.match(r'(.+)\s+([^ ]+)', name)
                if m:
                    print('''    .. {}:: {} {}::{}'''.format(
                        'function' if name.endswith(')') else self.member_domain,
                        m.group(1),
                        self.name,
                        m.group(2)))
                else:
                    if name.endswith(')'):
                        # For function, without return type, like
                        # constructor
                        print('''    .. {}:: {}::{}'''.format(
                            'function' if name.endswith(')') else self.member_domain,
                            self.name, name))
                    else:
                        # enum
                        print('''    .. {}:: {}'''.format(
                            'function' if name.endswith(')') else self.member_domain,
                            name))
                print()
                for line in content:
                    print('''        {}'''.format(line))
            print()

class MacroDoc:
    def __init__(self, name, content):
        self.name = name
        self.content = content

    def write(self, out):
        print('''.. macro:: {}'''.format(self.name))
        print()
        for line in self.content:
            print('    {}'.format(line))

def make_api_ref(infiles):
    macros = []
    enums = []
    types = []
    functions = []
    for infile in infiles:
        while True:
            line = infile.readline()
            if not line:
                break
            elif line == '/**\n':
                line = infile.readline()
                doctype = line.split()[1]
                if doctype == '@function':
                    functions.append(process_function('function', infile))
                if doctype == '@functypedef':
                    types.append(process_function('type', infile))
                elif doctype == '@typedef':
                    types.append(process_typedef(infile))
                elif doctype in ['@class', '@struct', '@union']:
                    types.append(process_struct(infile))
                elif doctype == '@enum':
                    enums.append(process_enum(infile))
                elif doctype == '@macro':
                    macros.append(process_macro(infile))
    alldocs = [('Macros', macros),
               ('Enums', enums),
               ('Types (classes, structs, unions and typedefs)', types),
               ('Functions', functions)]
    for title, docs in alldocs:
        if not docs:
            continue
        print(title)
        print('-'*len(title))
        for doc in docs:
            doc.write(sys.stdout)
            print()
        print()

def process_macro(infile):
    content = read_content(infile)
    line = infile.readline()
    macro_name = line.split()[1]
    return MacroDoc(macro_name, content)

def process_enum(infile):
    members = []
    enum_name = None
    content = read_content(infile)
    while True:
        line = infile.readline()
        if not line:
            break
        elif re.match(r'\s*/\*\*\n', line):
            member_content = read_content(infile)
            line = infile.readline()
            items = line.split()
            member_name = items[0].rstrip(',')
            if len(items) >= 3:
                member_content.insert(0, '(``{}``) '\
                                          .format(items[2].rstrip(',')))
            members.append((member_name, member_content))
        elif line.startswith('}'):
            if not enum_name:
                enum_name = line.rstrip().split()[1]
            enum_name = re.sub(r';$', '', enum_name)
            break
        elif not enum_name:
            m = re.match(r'^\s*enum\s+([\S]+)\s*{\s*', line)
            if m:
                enum_name = m.group(1)
    return StructDoc(enum_name, content, 'type', members, 'c:macro')

def process_struct(infile):
    members = []
    domain = 'type'
    struct_name = None
    content = read_content(infile)
    while True:
        line = infile.readline()
        if not line:
            break
        elif re.match(r'\s*/\*\*\n', line):
            member_content = read_content(infile)
            line = infile.readline()
            member_name = line.rstrip().rstrip(';')
            member_name = re.sub(r'\)\s*=\s*0', ')', member_name)
            member_name = re.sub(r' virtual ', '', member_name)
            members.append((member_name, member_content))
        elif line.startswith('}') or\
                (line.startswith('typedef ') and line.endswith(';\n')):
            if not struct_name:
                if line.startswith('}'):
                    index = 1
                else:
                    index = 3
                struct_name = line.rstrip().split()[index]
            struct_name = re.sub(r';$', '', struct_name)
            break
        elif not struct_name:
            m = re.match(r'^\s*(struct|class)\s+([\S]+)\s*(?:{|;)', line)
            if m:
                domain = m.group(1)
                if domain == 'struct':
                    domain = 'type'
                struct_name = m.group(2)
                if line.endswith(';\n'):
                    break
    return StructDoc(struct_name, content, domain, members, 'member')

def process_function(domain, infile):
    content = read_content(infile)
    func_proto = []
    while True:
        line = infile.readline()
        if not line:
            break
        elif line == '\n':
            break
        else:
            func_proto.append(line)
    func_proto = ''.join(func_proto)
    func_proto = re.sub(r';\n$', '', func_proto)
    func_proto = re.sub(r'\s+', ' ', func_proto)
    func_proto = re.sub(r'typedef ', '', func_proto)
    return FunctionDoc(func_proto, content, domain)

def process_typedef(infile):
    content = read_content(infile)
    lines = []
    while True:
        line = infile.readline()
        if not line:
            break
        elif line == '\n':
            break
        else:
            lines.append(line)
    typedef = ''.join(lines)
    typedef = re.sub(r';\n$', '', typedef)
    typedef = re.sub(r'\s+', ' ', typedef)
    typedef = re.sub(r'typedef ', '', typedef)
    return TypedefDoc(typedef.split()[-1], content)

def read_content(infile):
    content = []
    while True:
        line = infile.readline()
        if not line:
            break
        if re.match(r'\s*\*/\n', line):
            break
        else:
            content.append(transform_content(line.rstrip()))
    return content

def arg_repl(matchobj):
    return '*{}*'.format(matchobj.group(1).replace('*', '\\*'))

def transform_content(content):
    content = re.sub(r'^\s+\* ?', '', content)
    content = re.sub(r'\|([^\s|]+)\|', arg_repl, content)
    content = re.sub(r':enum:', ':macro:', content)
    return content

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate API reference")
    parser.add_argument('--header', type=argparse.FileType('rb', 0),
                        help='header inserted at the top of the page')
    parser.add_argument('files', nargs='+', type=argparse.FileType('rb', 0),
                        help='source file')
    args = parser.parse_args()
    if args.header:
        print(args.header.read())
    for infile in args.files:
        make_api_ref(args.files)
