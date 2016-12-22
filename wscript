# Author: Rémy Taymans
#         Louis Randriamora
# Version: Novembre 29, 2016

from os import listdir
from os.path import isfile
from os.path import splitext

flags = '-Wall -Werror'

def options(opt):
    opt.load('compiler_c')

def configure(cnf):
    cnf.load('compiler_c')

def build(bld):
    print("Compile sh6...")
    bld.program(source='sh6.c sh6lib.c', 
                target='sh6', 
                cflags=flags)
    if isfile('build/sh6'):
        print("Move sh6.")
        bld.exec_command('cp build/sh6 sh6')

    sources = [f for f in listdir("cmd") \
               if f.endswith('.c') and not f.endswith('lib.c')]
    for f in sources:
        f_no_ext = splitext(f)[0]
        f_lib = f_no_ext + "lib.c"

        print("Compile "+f_no_ext+"...")
        if isfile("cmd/"+f_lib):
            bld.program(source="cmd/"+f+" cmd/"+f_lib,
                        target="cmd/"+f_no_ext,
                        cflags=flags)
        else:
            bld.program(source="cmd/"+f,
                        target="cmd/"+f_no_ext,
                        cflags=flags)

        if isfile('build/cmd/'+f_no_ext):
            print("Move "+f_no_ext+".")
            bld.exec_command('cp build/cmd/'+f_no_ext+' cmd/'+f_no_ext)

