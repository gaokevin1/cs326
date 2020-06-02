#!/usr/bin/env bash

if ! ( which dot &> /dev/null ); then
    echo "ERROR: Couldn't find 'dot' -- to install on your VM, run: (as root)"
    echo
    echo "    pacman -Sy graphviz"
    echo
    exit 1
fi

if [[ "${#}" -ne 2 ]]; then
    echo "Usage: $(basename ${0}) mem-trace.mem output-file.png"
    exit 1
fi

script_loc="$(cd "$(dirname "$0")" && pwd)"
${script_loc}/mem2dot.bash "${1}" | dot -Tpng > "${2}"

