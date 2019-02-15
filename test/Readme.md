# Automatic tests

run_test.sh contains all automatic tests in the variable TEST.

One test has two lines: first the command and second its output. The output of
a test should be on one line only and not start with space.

The test programs should exercise one action defined in asa.h:

- window: apply window in `asa_pad_and_window()`
- power: distribute bins to lines in `asa_distribute_lines()`

Todo: lines (test whether bins are correctly combined to lines)