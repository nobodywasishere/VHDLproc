from setuptools import setup
from vhdlproc import vhdlproc


def readme():
    with open("README.md") as f:
        return f.read()


setup(
    name="vhdlproc",
    version=vhdlproc.__version__,
    description="A simple VHDL preprocessor",
    long_description=readme(),
    long_description_content_type="text/markdown",
    url="https://github.com/nobodywasishere/vhdlproc",
    author="Margret Riegert",
    author_email="margret@eowyn.net",
    license="GPLv3",
    packages=["vhdlproc"],
    package_data={"vhdlproc": ["tests/*"]},
    install_requires=[],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ],
    entry_points={"console_scripts": ["vhdlproc = vhdlproc.vhdlproc:_cli"]},
)
