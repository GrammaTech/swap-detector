#====- test_swappedargs.py -----------------------------------*- Python -*-===//
#
#  Copyright (C) 2020 GrammaTech, Inc.
#
#  This code is licensed under the MIT license. See the LICENSE file in the
#  project root for license terms.
#
# This material is based on research sponsored by the Department of Homeland
# Security (DHS) Office of Procurement Operations, S&T acquisition Division via
# contract number 70RSAT19C00000056. The views and conclusions contained herein
# are those of the authors and should not be interpreted as necessarily
# representing the official policies or endorsements, either expressed or
# implied, of the Department of Homeland Security.
#
#====----------------------------------------------------------------------===//
import swappedargs


def test_minimal():
    checker = swappedargs.Checker()
    results = checker.check_call(['hi', 'lo', 'foo', 'bar'])
    assert results == []


def test_multi_results():
    checker = swappedargs.Checker()
    results = checker.check_call(callee='bar',
                                 parameters=['hi', 'lo', 'foo', 'bar'],
                                 arguments=['lo', 'hi', 'bar', 'foo'],
                                 is_variadic=False)
    assert len(results) == 2
    assert results[0]['arg1'] == 1
    assert results[0]['arg2'] == 2
    assert isinstance(results[0]['morphemes1'], set)
    assert isinstance(results[0]['morphemes2'], set)

    assert results[1]['arg1'] == 3
    assert results[1]['arg2'] == 4
    assert isinstance(results[1]['morphemes1'], set)
    assert isinstance(results[1]['morphemes2'], set)
