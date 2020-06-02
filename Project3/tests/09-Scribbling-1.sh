source "${TEST_DIR}/funcs.bash"

expected=$(cat <<EOM
Printing uninitialized variables:
-1431655766
-1431655766
-1431655766
12297829382473034410
aaaaaaaa
aaaaaaaaaaaaaaaa
Totalling up uninitialized arrays:
850000
1700000
calloc should still zero out the memory:
0
EOM
)

test_start "Memory Scribbling"

# Check to make sure the library exists
[[ -e "./allocator.so" ]] || test_end 1

actual=$(LD_PRELOAD=./allocator.so tests/progs/scribble) || test_end

compare <(echo "${expected}") <(echo "${actual}")

test_end
