source "${TEST_DIR}/funcs.bash"

test_start "free() tests" \
    "Makes a large amount of random allocations and frees them"

# Check to make sure the library exists
[[ -e "./allocator.so" ]] || test_end 1

# If this crashes, you likely have an error in your free() / reuse logic that is
# causing memory to leak and eventually run out.
LD_PRELOAD=./allocator.so tests/progs/free

test_end
