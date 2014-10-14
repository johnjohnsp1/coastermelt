#!/usr/bin/env python

from distutils.core import setup, Extension

setup(ext_modules=[
	Extension('remote', ['remote.cpp'],
		extra_link_args = '-framework CoreFoundation -framework IOKit'.split(),
		include_dirs = ['../lib']
	)
])