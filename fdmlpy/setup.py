import distutils
from distutils.core import Extension, setup
package_data = {'@FDMLPY_PACKAGE_NAME@': ['__init__.py', '@FDMLPY_TARGET_LINKER_FILE@', '@FDML_TARGET_LINKER_FILE@', 'fdmlpy.pyi']}
setup(version='@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@.@CMAKE_PROJECT_VERSION_PATCH@',
      description='Few Distance Measurements Robot Localization Library',
      packages=['@FDMLPY_PACKAGE_NAME@'],
      # platforms=['any'],
      platforms=[distutils.util.get_platform().replace('-','_').replace('.','_')],
      author='Barak Ugav, Efi Fogel',
      author_email='barakugav@gmail.com',
      url='https://github.com/barakugav/FDML',
      package_dir={'@FDMLPY_PACKAGE_NAME@': '.'},
      package_data=package_data,
      license='proprietary',
      # NOTE: Create fake extension so that distutils builds a platform specific whl.
      ext_modules=[Extension('fake', sources=[])])
