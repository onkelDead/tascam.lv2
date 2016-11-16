#!/usr/bin/env python
from waflib.extras import autowaf as autowaf
import re

# Variables for 'waf dist'
APPNAME = 'tascam.lv2'
VERSION = '1.0.0'

# Mandatory variables
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_c')
    opt.load('lv2')

    autowaf.set_options(opt)

def configure(conf):
    conf.load('compiler_c')
    conf.load('lv2')
    autowaf.configure(conf)
    # autowaf.set_c99_mode(conf)
    autowaf.display_header('Tascam Configuration')

    if not autowaf.is_child():
        autowaf.check_pkg(conf, 'lv2', uselib_store='LV2')
        autowaf.check_pkg(conf, 'alsa', uselib_store='ASOUND')

    conf.check(features='c cshlib', lib='m', uselib_store='M', mandatory=False)
    conf.check(features='c cshlib', lib='asound', uselib_store='M', mandatory=False)

    autowaf.display_msg(conf, 'LV2 bundle directory', conf.env.LV2DIR)
    print('')

def build(bld):
    bundle = 'tascam.lv2'

    # Make a pattern for shared objects without the 'lib' prefix
    module_pat = re.sub('^lib', '', bld.env.cshlib_PATTERN)
    module_ext = module_pat[module_pat.rfind('.'):]

    # Build manifest.ttl by substitution (for portable lib extension)
    bld(features     = 'subst',
        source       = 'manifest.ttl.in',
        target       = '%s/%s' % (bundle, 'manifest.ttl'),
        install_path = '${LV2DIR}/%s' % bundle,
        LIB_EXT      = module_ext)

    # Copy other data files to build bundle (build/eg-amp.lv2)
    for i in ['tascam_comp_stereo.ttl tascam_comp.ttl tascam_eq.ttl tascam_eq_stereo.ttl']:
        bld(features     = 'subst',
            is_copy      = True,
            source       = i,
            target       = '%s/%s' % (bundle, i),
            install_path = '${LV2DIR}/%s' % bundle)

    # Use LV2 headers from parent directory if building as a sub-project
    includes = None
    if autowaf.is_child:
        includes = '../..'

    # Build plugin library
    obj = bld(features     = 'c cshlib',
              source       = 'tascam.c tascam_eq.c tascam_eq_stereo.c tascam_comp.c tascam_comp_stereo.c tascam_alsa.c',
              name         = 'tascam',
              target       = '%s/tascam' % bundle,
              install_path = '${LV2DIR}/%s' % bundle,
              uselib       = 'M LV2 -lasound',
              includes     = includes)


    obj.env.cshlib_PATTERN = module_pat
