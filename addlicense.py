#!/usr/bin/env python

import os, sys

license = """/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/"""

path = sys.argv[1] if len(sys.argv) > 1 else '.'

for root, _, files in os.walk(path):
    for file in files:
        if file.endswith('.cpp') or file.endswith('.h') or file.endswith('.hpp'):
            fname = os.path.join(root, file)
            with open(fname, 'rt') as inp:
                body = inp.read()
            stop = body.find('*/')
            if stop == -1 or body[:stop + 2] != license:
                print("Patching: " + fname)
                body = license + '\n' + body[stop + 2:].lstrip()
                with open(fname, 'wt') as out:
                    out.write(body)
