import sys
import os
import fnmatch

def walk_r(path, glb):
    for root, dirnames, filenames in os.walk(path):
      for filename in fnmatch.filter(filenames, glb):
          yield os.path.join(root, filename)

def get_boost_includes(path):
    for line in open(path).readlines():
        good = line.split('//', 1)[0]
        pound = good.strip().startswith('#')
        preproc = ''
        if pound:
            preproc = good.strip().split('#')[1].strip()
        inc_or_def = preproc.startswith('include') or preproc.startswith('define')
        if pound and inc_or_def and 'boost/' in good and '<' in good and '>' in good:
            try:
                line = line.split('<', 1)[1]
                line = line.split('>', 1)[0]
                yield line
            except Exception as ex:
                print line, ex
                exit(-1)
        elif pound and inc_or_def and 'boost/' in good and '"' in good:
            try:
                line = line.split('"', 1)[1]
                line = line.split('"', 1)[0]
                yield line
            except Exception as ex:
                print line, ex
                exit(-1)

def add_to_includes(includes, path):
    for boost_include in get_boost_includes(path):
        if boost_include not in includes:
            includes.add(boost_include)
            add_to_includes(includes, os.path.join(boost_dir, boost_include))

def rename_path(s):
    if 'boost/archive' in s:
        s = s.replace('boost/', 'Pothos/')
    elif 'boost/serialization' in s:
        s = s.replace('boost/', 'Pothos/')
    else:
        s = s.replace('boost/', 'Pothos/serialization/impl/')
    return s

def rename_string(s):
    if 'include' in s or 'define' in s:
        s = rename_path(s)

    if 'namespace' in s:
        s = s.replace('boost', 'Pothos')

    s = s.replace('boost::', 'Pothos::')
    s = s.replace('boost_', 'Pothos_')
    s = s.replace('BOOST_', 'POTHOS_')

    if s.strip() == 'boost':
        s = s.replace('boost', 'Pothos')

    return s

def rename_file(src, dst):
    dst = rename_path(dst)
    if not os.path.exists(os.path.dirname(dst)):
        os.makedirs(os.path.dirname(dst))
    out = ""
    for line in open(src).readlines():
        stuff = line.split('//', 1)
        out += rename_string(stuff[0])
        if len(stuff) == 2: out += '//' + stuff[1]
    open(dst, 'w').write(out)

if __name__ == '__main__':
    includes = set()

    boost_dir, eos_dir, dest_dir = sys.argv[1:]

    for hpp in walk_r(os.path.join(eos_dir, 'eos'), '*.*'):
        rename_file(hpp, os.path.join(dest_dir, 'include/Pothos/archive/eos', os.path.basename(hpp)))
        add_to_includes(includes, hpp)

    for cpp in ['libs/program_options/src/convert.cpp', 'libs/program_options/src/utf8_codecvt_facet.cpp']:
        cpp = os.path.join(boost_dir, cpp)
        rename_file(cpp, os.path.join(dest_dir, 'lib', 'po_'+os.path.basename(cpp)))
        add_to_includes(includes, cpp)

    for cpp in walk_r(os.path.join(boost_dir, 'libs/serialization/src/'), '*.*'):
        rename_file(cpp, os.path.join(dest_dir, 'lib', os.path.basename(cpp)))
        add_to_includes(includes, cpp)

    for sub in ['serialization', 'mpl', 'archive']:
        for hdr in walk_r(os.path.join(boost_dir, 'boost', sub), '*.*'):
            new_name = hdr.split(sub+'/', 1)[1]
            rename_file(hdr, os.path.join(dest_dir, 'include', 'boost', sub, new_name))

    for inc in includes:

        rename_file(os.path.join(boost_dir, inc), os.path.join(dest_dir, 'include', inc))

        #print inc
    print len(includes)
