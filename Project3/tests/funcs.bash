
test_file_name="$(basename "${0}")"
test_num="${test_file_name%%-*}"
test_pts="${test_file_name##*-}"
test_pts="${test_pts/.sh/}"
test_name="${test_file_name%-*}"
test_name="${test_name##*-}"
in_test=false

exec &> "${TEST_DIR}/test.${test_num}.md"
#exec &> "./grade-info.md"
#
#deductions=""
#
#return=0
#test_count=0
#
#echo "# Grade Information"
#echo

test_start() {
    if [[ ${in_test} == true ]]; then
        echo "FATAL: Entering test block failed: missing 'test_end'?"
        exit 1
    fi
    in_test=true
    ((test_count++))
    echo "## Test ${test_num}: ${1} [${test_pts} pts]"
    if [[ -n ${2} ]]; then
        echo
        echo "${2}"
    fi
    echo
    echo '```'
    trace_on
}

test_end() {
    return=${?}
    if [[ -n ${1} ]]; then
        return=${1}
    fi

    if [[ "${return}" -eq 139 ]]; then
        echo '--------------------------------------'
        echo 'ERROR: program terminated with SIGSEGV'
        echo '--------------------------------------'
    fi

    { trace_off; } 2> /dev/null
    in_test=false
    echo -e '```'"\n"
    exit "${return}"
}

trace_on() {
    set -v
}

trace_off() {
    { set +v; } 2> /dev/null
}

draw_sep() {
    local term_sz="$(tput cols)"
    local half=$(((term_sz - 1) / 2))
    local midpoint="${1}"
    if [[ -z "${midpoint}" ]]; then
        midpoint='-'
    fi

    for (( i = 0 ; i < half ; ++i )); do
        echo -n "-"
    done
    echo -n "${midpoint}"
    for (( i = 0 ; i < (( half - (term_sz % 2))); ++i )); do
        echo -n "-"
    done
    echo
}

compare() {
    echo
    local term_sz="$(tput cols)"
    local half=$(((term_sz - 1) / 2))
    printf "%-${half}s| %s\n" "Expected Output" "Actual Output"
    draw_sep 'V'
    sdiff --expand-tabs --width="${term_sz}" ${@}
    local result=${?}
    draw_sep '^'
    return ${result}
}

fake_tty() {
    timeout 5 script --flush --quiet --command "$(printf "%q " "$@")" /dev/null
}
