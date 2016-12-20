# Author: Rémy Taymans
#         Louis Randriamora
# Version: Novembre 29, 2016

from os import listdir
from os.path import splitext

flags = '-Wall -Werror'

def options(opt):
    opt.load('compiler_c')

def configure(cnf):
    cnf.load('compiler_c')

def build(bld):
    bld.program(source='sh6.c', 
                target='sh6', 
                cflags=flags)
    sources = [f for f in listdir("cmd") if f.endswith('.c')]
    for f in sources:
        bld.program(source="cmd/"+f,
                    target="cmd/"+splitext(f)[0],
                    cflags=flags)

