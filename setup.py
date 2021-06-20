from setuptools import setup

setup(
    name='vhdlproc',
    version='2.0',    
    description='A simple VHDL preprocessor',
    url='https://github.com/nobodywasishere/vhdlproc',
    author='Michael Riegert',
    author_email='michael@eowyn.net',
    license='GPLv3',
    packages=['vhdlproc'],
    package_data={'vhdlproc': ['tests/*']},
    install_requires=['argparse'],

    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',
        'Operating System :: OS Independent'
    ],

    entry_points={
        'console_scripts': [
            'vhdlproc = vhdlproc.vhdlproc:cli'
        ]
    },
)
