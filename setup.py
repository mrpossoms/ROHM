from distutils.core import setup, Extension

extra_compile_args = ["-Wall", "-Wextra", "-fPIC"]

module1 = Extension('rohm',
                    define_macros = [('MAJOR_VERSION', '0'),
                                     ('MINOR_VERSION', '1')],
                    extra_compile_args=extra_compile_args,
                    include_dirs = ['/usr/local/include', 'src'],
                    libraries = ['rohm', 'tiff'],
                    library_dirs = ['/usr/local/lib', 'lib/x86_64-linux-gnu'],
                    sources = ['src/python/rohmsubmodule.cpp'])

setup (name = 'ROHM',
       version = '0.1',
       description = 'Python module for using ROHM estimation algorithm',
       author = 'Kirk Roerig',
       author_email = 'kirk@kirkroerig.com',
       long_description = '''
Python module for using ROHM estimation algorithm
''',
       ext_modules = [module1])