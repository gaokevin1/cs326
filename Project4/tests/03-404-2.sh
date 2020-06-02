source "${TEST_DIR}/funcs.bash"

port=$(choose_port)

test_start "Serving a basic web page"

setsid $TEST_DIR/start_server "${port}" &
pid=${!}
wait_port "${port}"

actual=$(timeout 5 wget -O- "http://localhost:${port}/this/does/not/exist.html")
return="${?}"

stop_server ${pid}

# Return value will be 8 for a server error
[[ ${return} -eq 8 ]]

test_end
