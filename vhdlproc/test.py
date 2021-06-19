import os

from vhdlproc import VHDLproc

def test_file(name, identifiers={}):
    print(f'\n== Testing {name} ==')
    proc = VHDLproc()
    filename = os.path.dirname(__file__) + f'/../tests/{name}.vhdl'
    passed = True

    try:
        parsed = proc.parse_file(filename, identifiers=identifiers)
    except Exception as e:
        print(e)
        print("Failed")
        passed = passed and False

    if passed:
        print('== Passed ==')
    else:
        print('== Failed ==')

    return passed


def test_all():
    # return not (test_include() and test_and() and test_nest())
    return not (test_file('include') and test_file('and') and test_file('nest'))