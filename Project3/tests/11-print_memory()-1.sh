source "${TEST_DIR}/funcs.bash"


test_start "print_memory() function"

output=$(tests/progs/print-test 2> /dev/null)

echo "${output}"

# Must have 3 or more regions
regions=$(grep '\[REGION\]' <<< "${output}" | wc -l)
[[ ${regions} -ge 3 ]] || test_end 1

# Must have 3 or more allocations of req size 4000
# We also check that the request size is in column 6
blocks=$(grep '\[BLOCK\]' <<< "${output}" \
    | awk '{print $6}' \
    | grep '4000' \
    | wc -l)
[[ ${blocks} -ge 3 ]] || test_end 1

test_end
