from distutils.core import setup
package_data = {'': ['@FDMLPY_TARGET_LINKER_FILE@', 'fdmlpy.pyi']}
setup(name='fdmlpy',
      version='@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@',
      description='Few Distance Measurements Robot Localization Library',
      py_modules=['fdmlpy'],
      packages=[''],
      platforms=['any'],
      author='@FDML_PROJECT_AUTHORS@',
      author_email='@FDML_PROJECT_EMAILS@',
      url='@FDML_PROJECT_URL@',
      package_data=package_data,
      license='GPLv3+')
