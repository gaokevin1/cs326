source "${TEST_DIR}/funcs.bash"

expected_order=$(cat <<EOM
0
5
6
7
2
3
4
EOM
)

algo="first_fit"

test_start "Testing ${algo}"

# Check to make sure there were no extra pages
ALLOCATOR_ALGORITHM=${algo} \
tests/progs/allocations-2 2> /dev/null || test_end

output=$( \
    ALLOCATOR_ALGORITHM=${algo} \
    tests/progs/allocations-3 2> /dev/null)

echo "${output}"

# Just get the block ordering from the output. We ignore the last allocation
# that is caused by printing to stdout.
block_order=$(grep '\[BLOCK\]' <<< "${output}" \
    | sed 's/.*(\([0-9]*\)).*/\1/g' \
    | head --lines=-1)

compare <(echo "${expected_order}") <(echo "${block_order}") || test_end

test_end
