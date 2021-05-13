import unittest
import sys
from gi.repository import GLib


class TestGI(unittest.TestCase):
    def test_glib_enum(self):
        '''GLib enum'''
        self.assertEqual(GLib.IOCondition.IN.value_nicks, ['in'])

    def test_glib_flag(self):
        '''GLib flag'''
        self.assertEqual(GLib.IOFlags.IS_READABLE.value_nicks, ['is_readable'])

    def test_method(self):
        '''GLib method call'''

        self.assertEqual(GLib.find_program_in_path('bash'), '/bin/bash')

unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))
