source "${TEST_DIR}/funcs.bash"

port=$(choose_port)

test_start "Serving a basic web page"

setsid $TEST_DIR/start_server "${port}" &
pid=${!}
wait_port "${port}"

expected=$(cat "${TEST_DIR}/html/test.html")
actual=$(timeout 5 wget -O- "http://localhost:${port}/test.html")

stop_server ${pid}

compare <(echo "${expected}") <(echo "${actual}")

test_end
