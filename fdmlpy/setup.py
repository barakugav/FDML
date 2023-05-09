from distutils.core import setup
package_data = {'': ['@FDMLPY_TARGET_LINKER_FILE@', 'fdmlpy.pyi']}
setup(name='fdmlpy',
      version='@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@',
      description='Few Distance Measurements Robot Localization Library',
      py_modules=['fdmlpy'],
      packages=[''],
      platforms=['any'],
      author='Barak Ugav, Efi Fogel',
      author_email='barakugav@gmail.com',
      url='https://github.com/barakugav/FDML',
      package_data=package_data,
      license='')
