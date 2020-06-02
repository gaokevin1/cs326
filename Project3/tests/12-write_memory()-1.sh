source "${TEST_DIR}/funcs.bash"


test_start "write_memory() function"

rm -f test-memory-output-file.txt

tests/progs/write-test test-memory-output-file.txt 2> /dev/null

output=$(cat test-memory-output-file.txt)

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
