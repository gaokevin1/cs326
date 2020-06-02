## Test 00: Invalid /proc directory [1 pts]

If the directory given for -p is invalid, print an error message and exit.
Return EXIT_FAILURE or similar. Test fails if return = 0 or segfault.

```

./inspector -a -p /this/directory/does/not/exist
++ ./inspector -a -p /this/directory/does/not/exist
inspector.c:125:main(): Using alternative proc directory: /this/directory/does/not/exist
inspector.c:147:main(): Options selected: hardware system task_list task_summary
System Information 
-------------------
Hostname: 
Kernel Version: 
Uptime: 

Hardware Information 
-------------------
CPU Model: 
Processing Units: 

Task Information 
-------------------
Since boot: 
    Interrupts: (null)
    Context Switches: (null)
    Forks: (null)

  PID |        State |                 Task Name |            User | Tasks 
[[ ${?} -ne 0 && ${?} -ne 139 ]];
++ [[ 0 -ne 0 ]]
test_end
++ test_end
++ return=1
```

## Test 01: System/Hardware/Task Headers [1 pts]

```
output_text=$(./inspector -a)
./inspector -a
+++ ./inspector -a
inspector.c:147:main(): Options selected: hardware system task_list task_summary
++ output_text='System Information 
-------------------
Hostname: 
Kernel Version: 
Uptime: 

Hardware Information 
-------------------
CPU Model: 
Processing Units: 

Task Information 
-------------------
Since boot: 
    Interrupts: (null)
    Context Switches: (null)
    Forks: (null)

  PID |        State |                 Task Name |            User | Tasks '
grep -q 'System Information' <<< "${output_text}" \
    && grep -q 'Hardware Information' <<< "${output_text}" \
    && grep -q 'Task Information' <<< "${output_text}" \
    && sed 's/[[:space:]]//g' <<< "${output_text}" | grep -q 'PID|State|TaskName|User|Tasks'
++ grep -q 'System Information'
++ grep -q 'Hardware Information'
++ grep -q 'Task Information'
++ sed 's/[[:space:]]//g'
++ grep -q 'PID|State|TaskName|User|Tasks'
test_end
++ test_end
++ return=0
```

## Test 02: Hostname Check [1 pts]

```
reported_hostname=$(./inspector -p "${TEST_DIR}/fakeproc" -s \
    | grep '^Hostname:' | awk '{print $2}')
./inspector -p "${TEST_DIR}/fakeproc" -s     | grep '^Hostname:' | awk '{print $2}'
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -s
+++ grep '^Hostname:'
+++ awk '{print $2}'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: system 
++ reported_hostname=
expected_hostname="deus-ex-machina"
++ expected_hostname=deus-ex-machina

actual_hostname=$(./inspector -s \
    | grep '^Hostname:' | awk '{print $2}')
./inspector -s     | grep '^Hostname:' | awk '{print $2}'
+++ ./inspector -s
+++ grep '^Hostname:'
+++ awk '{print $2}'
inspector.c:147:main(): Options selected: system 
++ actual_hostname=
expected_actual_hostname=$(hostname)
hostname
+++ hostname
++ expected_actual_hostname=Kevin-Gaos-rMBP.local

[ "${reported_hostname}" = "${expected_hostname}" ] \
    && [ "${actual_hostname}" = "${expected_actual_hostname}" ]
++ '[' '' = deus-ex-machina ']'
test_end
++ test_end
++ return=1
```

## Test 03: Kernel Version [1 pts]

```
reported_version=$(./inspector -p "${TEST_DIR}/fakeproc" -s \
    | grep -i '^Kernel Version:' | awk '{print $3}')
./inspector -p "${TEST_DIR}/fakeproc" -s     | grep -i '^Kernel Version:' | awk '{print $3}'
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -s
+++ grep -i '^Kernel Version:'
+++ awk '{print $3}'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: system 
++ reported_version=
expected_version="1.33.7-111.1.1.el7.x86_64"
++ expected_version=1.33.7-111.1.1.el7.x86_64

actual_version=$(./inspector -s | grep -i '^Kernel Version:' | awk '{print $3}')
./inspector -s | grep -i '^Kernel Version:' | awk '{print $3}'
+++ ./inspector -s
+++ grep -i '^Kernel Version:'
+++ awk '{print $3}'
inspector.c:147:main(): Options selected: system 
++ actual_version=
expected_actual_version=$(uname -r)
uname -r
+++ uname -r
++ expected_actual_version=17.7.0

[ "${reported_version}" = "${expected_version}" ] \
    && [ "${actual_version}" = "${expected_actual_version}" ]
++ '[' '' = 1.33.7-111.1.1.el7.x86_64 ']'
test_end
++ test_end
++ return=1
```

## Test 04: Basic Uptime Formatting [1 pts]

```
reported_uptime=$(./inspector -p "${TEST_DIR}/fakeproc" -s \
    | grep -i '^Uptime:' | tr -s ' ')
./inspector -p "${TEST_DIR}/fakeproc" -s     | grep -i '^Uptime:' | tr -s ' '
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -s
+++ grep -i '^Uptime:'
+++ tr -s ' '
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: system 
++ reported_uptime=
expected_uptime="Uptime: 17 days, 5 hours, 38 minutes, 55 seconds"
++ expected_uptime='Uptime: 17 days, 5 hours, 38 minutes, 55 seconds'
[ "${reported_uptime}" = "${expected_uptime}" ]
++ '[' '' = 'Uptime: 17 days, 5 hours, 38 minutes, 55 seconds' ']'
test_end
++ test_end
++ return=1
```

## Test 05: Uptime Formatting: Short Timespan [1 pts]

```
prev_uptime=$(cat "${TEST_DIR}/fakeproc/uptime")
cat "${TEST_DIR}/fakeproc/uptime"
+++ cat /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/uptime
++ prev_uptime='1489135.87 93892480.11'
echo "135.17 93892480.110" > "${TEST_DIR}/fakeproc/uptime"
++ echo '135.17 93892480.110'
reported_uptime=$(./inspector -p "${TEST_DIR}/fakeproc" -s \
    | grep -i '^Uptime:' | tr -s ' ')
./inspector -p "${TEST_DIR}/fakeproc" -s     | grep -i '^Uptime:' | tr -s ' '
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -s
+++ grep -i '^Uptime:'
+++ tr -s ' '
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: system 
++ reported_uptime=
echo "${prev_uptime}" > "${TEST_DIR}/fakeproc/uptime"
++ echo '1489135.87 93892480.11'
expected_uptime="Uptime: 2 minutes, 15 seconds"
++ expected_uptime='Uptime: 2 minutes, 15 seconds'
[ "${reported_uptime}" = "${expected_uptime}" ]
++ '[' '' = 'Uptime: 2 minutes, 15 seconds' ']'
test_end
++ test_end
++ return=1
```

## Test 06: Uptime Formatting: Long Timespan [1 pts]

```
prev_uptime=$(cat "${TEST_DIR}/fakeproc/uptime")
cat "${TEST_DIR}/fakeproc/uptime"
+++ cat /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/uptime
++ prev_uptime='1489135.87 93892480.11'
echo "126146404.41 93892480.110" > "${TEST_DIR}/fakeproc/uptime"
++ echo '126146404.41 93892480.110'
reported_uptime=$(./inspector -p "${TEST_DIR}/fakeproc" -s \
    | grep -i '^Uptime:' | tr -s ' ')
./inspector -p "${TEST_DIR}/fakeproc" -s     | grep -i '^Uptime:' | tr -s ' '
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -s
+++ grep -i '^Uptime:'
+++ tr -s ' '
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: system 
++ reported_uptime=
echo "${prev_uptime}" > "${TEST_DIR}/fakeproc/uptime"
++ echo '1489135.87 93892480.11'
expected_uptime="Uptime: 4 years, 40 minutes, 4 seconds"
++ expected_uptime='Uptime: 4 years, 40 minutes, 4 seconds'
[ "${reported_uptime}" = "${expected_uptime}" ]
++ '[' '' = 'Uptime: 4 years, 40 minutes, 4 seconds' ']'
test_end
++ test_end
++ return=1
```

## Test 07: Processing Units [1 pts]

```

reported_units=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'Processing Units:' | awk '{print $3}')
./inspector -p "${TEST_DIR}/fakeproc" -r     | grep -i 'Processing Units:' | awk '{print $3}'
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
+++ grep -i 'Processing Units:'
+++ awk '{print $3}'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
++ reported_units=
expected_units=64
++ expected_units=64

actual_units=$(./inspector -r | grep -i 'Processing Units:' | awk '{print $3}')
./inspector -r | grep -i 'Processing Units:' | awk '{print $3}'
+++ ./inspector -r
+++ grep -i 'Processing Units:'
+++ awk '{print $3}'
inspector.c:147:main(): Options selected: hardware 
++ actual_units=
expected_actual_units=$(grep '^processor' /proc/cpuinfo | wc -l)
grep '^processor' /proc/cpuinfo | wc -l
+++ grep '^processor' /proc/cpuinfo
+++ wc -l
grep: /proc/cpuinfo: No such file or directory
++ expected_actual_units='       0'

[ ${reported_units} -eq ${expected_units} ] \
    && [ ${actual_units} -eq ${expected_actual_units} ]
++ '[' -eq 64 ']'
./tests/07-Proc-Units-1.sh: line 12: [: -eq: unary operator expected
test_end
++ test_end
++ return=2
```

## Test 08: CPU Model [1 pts]

```

./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'CPU Model:.*AMD EPYC 7281 16-Core Processor'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
++ grep -i 'CPU Model:.*AMD EPYC 7281 16-Core Processor'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
model_correct=${?}
++ model_correct=1

actual_model=$(./inspector -r | grep -i 'CPU Model:' | sed 's/.*:\s*//')
./inspector -r | grep -i 'CPU Model:' | sed 's/.*:\s*//'
+++ ./inspector -r
+++ grep -i 'CPU Model:'
+++ sed 's/.*:\s*//'
inspector.c:147:main(): Options selected: hardware 
++ actual_model=
expected_actual_model=$(grep 'model name' /proc/cpuinfo \
    | sed 's/.*: //' | head -n 1 )
grep 'model name' /proc/cpuinfo     | sed 's/.*: //' | head -n 1 
+++ grep 'model name' /proc/cpuinfo
+++ sed 's/.*: //'
+++ head -n 1
grep: /proc/cpuinfo: No such file or directory
++ expected_actual_model=

[ ${model_correct} -eq 0 ] \
    && [ "${actual_model}" == "${expected_actual_model}" ]
++ '[' 1 -eq 0 ']'
test_end
++ test_end
++ return=1
```

## Test 09: Load Average [1 pts]

```

reported_load=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'Load Average' | awk '{print $(NF - 2), $(NF - 1), $(NF)}')
./inspector -p "${TEST_DIR}/fakeproc" -r     | grep -i 'Load Average' | awk '{print $(NF - 2), $(NF - 1), $(NF)}'
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
+++ grep -i 'Load Average'
+++ awk '{print $(NF - 2), $(NF - 1), $(NF)}'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
++ reported_load=
expected_load="1.87 1.27 1.07"
++ expected_load='1.87 1.27 1.07'

#actual_load=$(./inspector -r | grep -i 'Load Average' \
#    | awk '{print $(NF - 2), $(NF - 1), $(NF)}')
#expected_actual_load=$(awk '{ print $1, $2, $3 }' /proc/loadavg)

[ "${reported_load}" = "${expected_load}" ]
++ '[' '' = '1.87 1.27 1.07' ']'
test_end
++ test_end
++ return=1
```

## Test 10: Memory: 0%, 50% Usage [1 pts]

Checks the output and progress bar when memory usage is 0% and 50%.

```

cp "${TEST_DIR}/mem/zero" "${TEST_DIR}/fakeproc/meminfo"
++ cp /Users/kevingao/USF/cs326/Projects/Project1/tests/mem/zero /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/meminfo
# Spaces are removed before doing the string comparison:
./inspector -p "${TEST_DIR}/fakeproc" -r | sed 's/[[:space:]]//g' \
    | grep -i 'MemoryUsage:\[--------------------\]0.0%(0.0GB/5....GB)'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
++ sed 's/[[:space:]]//g'
++ grep -i 'MemoryUsage:\[--------------------\]0.0%(0.0GB/5....GB)'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
result1=${?}
++ result1=1
cp "${TEST_DIR}/mem/50" "${TEST_DIR}/fakeproc/meminfo"
++ cp /Users/kevingao/USF/cs326/Projects/Project1/tests/mem/50 /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/meminfo
./inspector -p "${TEST_DIR}/fakeproc" -r | sed 's/[[:space:]]//g' \
    | grep -i 'MemoryUsage:\[##########----------\]50.0%(2....GB/5....GB)'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
++ sed 's/[[:space:]]//g'
++ grep -i 'MemoryUsage:\[##########----------\]50.0%(2....GB/5....GB)'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
result2=${?}
++ result2=1
cp "${TEST_DIR}/mem/original" "${TEST_DIR}/fakeproc/meminfo"
++ cp /Users/kevingao/USF/cs326/Projects/Project1/tests/mem/original /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/meminfo
[ ${result1} -eq 0 ] && [ ${result2} -eq 0 ]
++ '[' 1 -eq 0 ']'
test_end
++ test_end
++ return=1
```

## Test 11: Memory: 94.9%, 100% Usage [1 pts]

Checks the output and progress bar when memory usage is 94.9% and 100%.

```

cp "${TEST_DIR}/mem/95" "${TEST_DIR}/fakeproc/meminfo"
++ cp /Users/kevingao/USF/cs326/Projects/Project1/tests/mem/95 /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/meminfo
./inspector -p "${TEST_DIR}/fakeproc" -r | sed 's/[[:space:]]//g' \
    | grep -i 'MemoryUsage:\[##################--\]94.9%(4....GB/5....GB)'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
++ sed 's/[[:space:]]//g'
++ grep -i 'MemoryUsage:\[##################--\]94.9%(4....GB/5....GB)'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
result1=${?}
++ result1=1

cp "${TEST_DIR}/mem/100" "${TEST_DIR}/fakeproc/meminfo"
++ cp /Users/kevingao/USF/cs326/Projects/Project1/tests/mem/100 /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/meminfo
# Spaces are removed before doing the string comparison:
./inspector -p "${TEST_DIR}/fakeproc" -r | sed 's/[[:space:]]//g' \
    | grep -i 'MemoryUsage:\[####################\]100.0%(5....GB/5....GB)'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
++ sed 's/[[:space:]]//g'
++ grep -i 'MemoryUsage:\[####################\]100.0%(5....GB/5....GB)'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 
result2=${?}
++ result2=1


cp "${TEST_DIR}/mem/original" "${TEST_DIR}/fakeproc/meminfo"
++ cp /Users/kevingao/USF/cs326/Projects/Project1/tests/mem/original /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc/meminfo
[ ${result1} -eq 0 ] && [ ${result2} -eq 0 ]
++ '[' 1 -eq 0 ']'
test_end
++ test_end
++ return=1
```

## Test 12: CPU Usage [1 pts]

CPU usage should register as 0% when both samples are identical

```

./inspector -p "${TEST_DIR}/fakeproc" -r | sed 's/[[:space:]]//g' \
    | grep -i 'CPUUsage:\[--------------------\]0.0%'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -r
++ sed 's/[[:space:]]//g'
++ grep -i 'CPUUsage:\[--------------------\]0.0%'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: hardware 

test_end
++ test_end
++ return=1
```

## Test 13: Task Count [1 pts]

```

tasks=$(./inspector -p "${TEST_DIR}/fakeproc" -t | grep -i 'tasks' \
    | awk -F': ' '{ print $2 }')
./inspector -p "${TEST_DIR}/fakeproc" -t | grep -i 'tasks'     | awk -F': ' '{ print $2 }'
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -t
+++ grep -i tasks
+++ awk '-F: ' '{ print $2 }'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_summary
++ tasks=

[ "${tasks}" = "1487" ]
++ '[' '' = 1487 ']'
test_end
++ test_end
++ return=1
```

## Test 14: Interrupts, Context Switches, and Forks [1 pts]

```

stats=$(./inspector -p "${TEST_DIR}/fakeproc" -t \
    | grep -i -e 'interrupts' -e 'context' -e 'forks' \
    | awk -F': ' '{ print $2 }')
./inspector -p "${TEST_DIR}/fakeproc" -t     | grep -i -e 'interrupts' -e 'context' -e 'forks'     | awk -F': ' '{ print $2 }'
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -t
+++ grep -i -e interrupts -e context -e forks
+++ awk '-F: ' '{ print $2 }'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_summary
++ stats=

stats="${stats//[$'\n']/ }"
++ stats=

[ "${stats}" = "2896141909 3012345539 3012654" ]
++ '[' '' = '2896141909 3012345539 3012654' ']'
test_end
++ test_end
++ return=1
```

## Test 15: Task List Size (should contain 1489/1490 lines for fakeproc) [1 pts]

```

size=$(./inspector -p "${TEST_DIR}/fakeproc" -l | wc -l)
./inspector -p "${TEST_DIR}/fakeproc" -l | wc -l
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -l
+++ wc -l
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_list 
++ size='       0'

[ "${size}" = "1489" ] || [ "${size}" = "1490" ]
++ '[' '       0' = 1489 ']'
++ '[' '       0' = 1490 ']'
test_end
++ test_end
++ return=1
```

## Test 16: Correct Process Name Truncation [1 pts]

```

./inspector -p "${TEST_DIR}/fakeproc" -l \
    | grep '88888' | sed 's/[[:space:]]//g' \
    | grep '|really_really_long_named_|'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -l
++ grep 88888
++ sed 's/[[:space:]]//g'
++ grep '|really_really_long_named_|'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_list 

test_end
++ test_end
++ return=1
```

## Test 17: Correct task list contents and sub-task count (9256) [1 pts]

```

./inspector -p "${TEST_DIR}/fakeproc" -l \
    | grep '88888' | sed 's/[[:space:]]//g' \
    | grep '9256$'
++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -l
++ grep 88888
++ sed 's/[[:space:]]//g'
++ grep '9256$'
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_list 

test_end
++ test_end
++ return=1
```

## Test 18: Task States [1 pts]

```

matches=$(./inspector -p "${TEST_DIR}/fakeproc" -l \
    | grep 'tracing stop' | wc -l)
./inspector -p "${TEST_DIR}/fakeproc" -l     | grep 'tracing stop' | wc -l
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -l
+++ grep 'tracing stop'
+++ wc -l
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_list 
++ matches='       0'

list=$(./inspector -p "${TEST_DIR}/fakeproc" -l)
./inspector -p "${TEST_DIR}/fakeproc" -l
+++ ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc -l
inspector.c:125:main(): Using alternative proc directory: /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
inspector.c:147:main(): Options selected: task_list 
++ list=

# Now check for each process state (running, sleeping, disk sleep, zombie, dead)
[ ${matches} -eq 1 ] \
    && grep 'sleeping' <<< "${list}" &>/dev/null \
    && grep 'running' <<< "${list}" &>/dev/null \
    && grep 'zombie' <<< "${list}" &>/dev/null
++ '[' 0 -eq 1 ']'
test_end
++ test_end
++ return=1
```

## Test 19: Memory Leak Check [1 pts]

```

valgrind ./inspector -p "${TEST_DIR}/fakeproc" 2>&1 \
    | grep 'no leaks are possible'
++ valgrind ./inspector -p /Users/kevingao/USF/cs326/Projects/Project1/tests/fakeproc
++ grep 'no leaks are possible'

test_end
++ test_end
++ return=1
```

