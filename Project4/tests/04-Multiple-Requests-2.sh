source "${TEST_DIR}/funcs.bash"

port=$(choose_port)

expected=$(cat <<EOM
index.html
mobile.css
print.css
syllabus
EOM
)

test_start "Serving a basic web page"

setsid $TEST_DIR/start_server "${port}" &
pid=${!}
wait_port "${port}"

rm -rf "${TEST_DIR}/multi"
timeout 5 wget --recursive -P "${TEST_DIR}/multi" "http://localhost:${port}/"
actual=$(ls -1 "${TEST_DIR}/multi/"*"/" | sort)

stop_server ${pid}

compare <(echo "${expected}") <(echo "${actual}")

test_end
