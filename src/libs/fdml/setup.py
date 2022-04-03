from distutils.core import setup
package_data = {'': ['fdml.so', 'fdml.pyi']}
setup(name='fdml',
      version='1.0',
      py_modules=['fdml'],
      packages=[''],
      author='Barak Ugav',
      author_email='barakugav@gmail.com',
      url='https://barakugav.com',
      package_data=package_data)
