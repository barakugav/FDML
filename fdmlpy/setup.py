import setuptools
import sysconfig
from setuptools import Extension, setup

# Only on Windows platform @FDML_TARGET_LINKER_FILE@ is substituted with
# the name of the fdml shared library file (i.e., fdml.dll). On othe platforms
# is becomes the empty string.
package_data = {'@FDMLPY_PACKAGE_NAME@':
                ['__init__.py',
                 '@FDMLPY_TARGET_LINKER_FILE@',
                 '@FDML_TARGET_LINKER_FILE@',
                 'fdmlpy.pyi']}

# Use a custom bdist_wheel to force non-pure wheel file aka no 'pyX-none-any'
try:
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
    class bdist_wheel(_bdist_wheel):
        def finalize_options(self):
            _bdist_wheel.finalize_options(self)
            self.root_is_pure = False
except ImportError:
    bdist_wheel = None

setup(version='@CMAKE_PROJECT_VERSION_MAJOR@.@CMAKE_PROJECT_VERSION_MINOR@.@CMAKE_PROJECT_VERSION_PATCH@',
      description='Few Distance Measurements Robot Localization Library',
      packages=['@FDMLPY_PACKAGE_NAME@'],
      # platforms=['any'],
      platforms=[sysconfig.get_platform().replace('-', '_').replace('.', '_')],
      author='Barak Ugav, Efi Fogel',
      author_email='barakugav@gmail.com',
      url='https://github.com/barakugav/FDML',
      package_dir={'@FDMLPY_PACKAGE_NAME@': '.'},
      package_data=package_data,
      license='proprietary',
      cmdclass={'bdist_wheel': bdist_wheel})
