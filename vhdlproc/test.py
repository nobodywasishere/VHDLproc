import os

from vhdlproc import VHDLproc

def test_include():
    print('\n== Testing include ==')
    proc = VHDLproc()
    test_file = os.path.dirname(__file__) + '/../tests/include.vhdl'
    passed = True

    print('\n  == Not including file ==')
    try:
        parsed = proc.parse_file(test_file)
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if parsed.count('\n') == 4:
        print('  Passed')
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print('\n  == Including file ==')
    try:
        parsed = proc.parse_file(test_file, identifiers={'include_file': 'true'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if parsed.count('\n') == 11:
        print('  Passed')
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    return passed

def test_and():
    print('\n== Testing and ==')
    proc = VHDLproc()
    test_file = os.path.dirname(__file__) + '/../tests/and.vhdl'
    passed = True

    print("\n  == a = 'a' and b = 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'a', 'b': 'b'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if "a = 'a' and b = 'b'" in parsed.split('\n'):
        print("  Passed")
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print("\n  == a /= 'a' and b = 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'c', 'b': 'b'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if "a /= 'a' and b = 'b'" in parsed.split('\n'):
        print("  Passed")
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print("\n  == a = 'a' and b /= 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'a', 'b': 'c'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if "a = 'a' and b /= 'b'" in parsed.split('\n'):
        print("  Passed")
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print("\n  == a /= 'a' and b /= 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'c', 'b': 'c'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if "a /= 'a' and b /= 'b'" in parsed.split('\n'):
        print("  Passed")
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    return passed

def test_nest():
    print('\n== Testing nested ifs ==')
    proc = VHDLproc()
    test_file = os.path.dirname(__file__) + '/../tests/nest.vhdl'
    passed = True

    print("\n  == a = 'a' and b = 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'a', 'b': 'b'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if parsed.count('--') == 6:
        print('  Passed')
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print("\n  == a /= 'a' and b = 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'c', 'b': 'b'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if parsed.count('--') == 7:
        print('  Passed')
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print("\n  == a = 'a' and b /= 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'a', 'b': 'c'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if parsed.count('--') == 6:
        print('  Passed')
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    print("\n  == a /= 'a' and b /= 'b' ==")
    try:
        parsed = proc.parse_file(test_file, identifiers={'a': 'c', 'b': 'c'})
    except Exception as e:
        print(e)
        print('  Failed')
        passed = passed and False
    if parsed.count('--') == 7:
        print('  Passed')
        passed = passed and True
    else:
        print('  Failed')
        passed = passed and False

    return passed

def test_all():
    return not (test_include() and test_and() and test_nest())