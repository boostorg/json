import gdb
import re
import traceback

def match(pattern, string):
    return re.fullmatch(r'\$\d+ = %s\n' % pattern, string, flags=re.M)

def test(label, expr, pattern):
    bp = gdb.Breakpoint('main:%s' % label)
    def do_test():
        output = gdb.execute('print %s' % expr, to_string=True)
        try:
            assert match(pattern, output)
        except:
            print(' Pattern: %s\n Output: %s' % (pattern, output))
            raise
    return do_test

def set_up_tests(objfile):
    # dir = os.path.dirname(objfile.filename)
    # gdb.execute('add-auto-load-safe-path %s' % dir)
    # gdb.execute('add-auto-load-scripts-directory %s' % dir)
    return [
        test('STRING1', 'js', '""'),
        test('STRING2', 'js', '"1"'),
        test('STRING3', 'js', '"this is a very long string, unusually long even, definitely not short"'),
    ]

return_code = 0
try:
    assert gdb.objfiles()
    tests = set_up_tests( gdb.objfiles()[0] )
    gdb.execute('start')
    gdb.execute('continue')
    for test in tests:
        test()
        gdb.execute('continue')
except BaseException:
    traceback.print_exc()
    gdb.execute('disable breakpoints')
    try:
        gdb.execute('continue')
    except:
        pass
    return_code = 1

gdb.execute('quit %s' % return_code)
