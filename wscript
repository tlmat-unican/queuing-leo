#!/usr/bin/env python
# encoding: utf-8

import os
import shutil

APPNAME = 'waf-project'
VERSION = '1.0'

top = '.'
out = 'build'

PROGS = 'programs'


def GetFiles(folder, extension):
    fns = [f for f in os.listdir(folder) if f.endswith(extension)]
    return fns


def options(ctx):
    ctx.load('compiler_cxx')
    ctx.add_option('-b', help=('Shortcut for build'),
                   type="string", default='', dest='build')

    ctx.add_option('-r', help=('Shortcut for run'),
                   type="string", default='', dest='run')

    ctx.add_option('-c', help=('Shortcut for configure'),
                   type="string", default='', dest='configure')

    ctx.add_option('--mode', default='debug', action='store',
                   help='execute the program after it is built')

    ctx.add_option('--program', action='store',
                   help='execute the program after it is built')


def configure(ctx):
    ctx.load('compiler_cxx')
    env = ctx.env
    ctx.env.append_value('LDFLAGS', ['-lstdc++fs'])
    ctx.env.append_value('LINKFLAGS', [])


def pre(ctx):
    if ctx.options.mode == 'release':
        ctx.env.CXXFLAGS = ['-O3', '-Wall', '-Wextra', '-std=c++17']
        log_define = []
    else:
        ctx.env.CXXFLAGS = ['-g', '-Wall', '-Wextra', '-std=c++17']
        log_define = ['LOG_ENABLED']      


def build(ctx):
    ctx.add_pre_fun(pre)
    print ('building')
    ctx.recurse('src')

    files = GetFiles(PROGS, '.cpp')
    for fn in files:
        mode = ctx.options.mode
        source = PROGS + '/' + fn
        target = '../' + PROGS + '/' + mode + '/' + os.path.splitext(fn)[0]
        if ctx.options.mode == 'debug':
            log_define = ['LOG_ENABLED']
        else:
            log_define = []
        ctx(
            features='cxx cxxprogram',
            target=target,
            source=source,
            defines=log_define,
            use='src'
        )


def distclean(ctx):
    """
    Cleans up more thoroughly (by deleting the build directory), 
    requiring configure again.
    """
    try:
        shutil.rmtree('build')
    except OSError, e:
        print ("INFO: %s - %s." % (e.filename, e.strerror))
    try:
        shutil.rmtree(PROGS + '/debug')
    except OSError, e:
        print ("INFO: %s - %s." % (e.filename, e.strerror))
    try:
        shutil.rmtree(PROGS + '/release')
    except OSError, e:
        print("INFO: %s - %s." % (e.filename, e.strerror))
    try:
        os.system('rm -rf .waf*')
    except OSError, e:
        print("INFO: %s - %s." % (e.filename, e.strerror))
    try:
        os.system('rm .lock*')
    except OSError, e:
        print ("INFO: %s - %s." % (e.filename, e.strerror))
