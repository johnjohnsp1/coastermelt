#!/usr/bin/env python

from distutils.core import setup, Extension

setup(ext_modules=[

	# Main I/O module that handles our SCSI communications quickly and recklesly
	Extension('remote', ['remote.cpp'],
		extra_link_args = '-framework CoreFoundation -framework IOKit'.split(),
		include_dirs = ['../lib']
	),

	# Pure C++ extension module for hilbert curve math
	Extension('hilbert', ['hilbert.cpp'])
])