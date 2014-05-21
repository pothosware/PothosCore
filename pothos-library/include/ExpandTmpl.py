# Copyright (c) 2014-2014 Josh Blum
# SPDX-License-Identifier: BSL-1.0

import os
import sys
from Cheetah.Template import Template

MAX_ARGS = 10

def expand(t, n):
    out = list()
    for i in range(n): out.append(t.replace('%d', str(i)))
    tmpl = ', '.join(out)
    if not tmpl: return ''
    return str(Template(tmpl))

def optarg(t, n):
    if n == 0: t = '/*'+t+'*/'
    return t

def cleanup(code):
    code = code.replace('template <>', 'inline')
    code = code.replace('<, ', '<')
    code = code.replace(', >', '>')
    code = code.replace(', )', ')')
    code = code.replace('(, ', '(')
    code = code.replace('\\#', '#')
    return code

if __name__ == '__main__':
    in_path = sys.argv[1]
    out_path = sys.argv[2]
    tmpl = open(in_path, 'r').read()
    for key in ['define', 'include', 'if', 'endif', 'else', 'ifdef', 'ifndef', 'pragma']:
        tmpl = tmpl.replace('#%s'%key, '\\#%s'%key)
    tmpl = tmpl.replace('#cond', '#if')
    code = str(Template(tmpl, dict(
        MAX_ARGS=MAX_ARGS, expand=expand, optarg=optarg,
    )))
    code = cleanup(code)
    if True:
    #if not os.path.exists(out_path) or open(out_path, 'r').read() != code:
        print 'write code to', out_path
        open(out_path, 'w').write(code)
