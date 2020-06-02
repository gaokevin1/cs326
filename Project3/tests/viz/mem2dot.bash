#!/usr/bin/env bash

regions=0
current_region=""
current_node=""
firsts=""
subgraph=""
outer_links="Head->"

if [[ ${#} -lt 1 ]]; then
    echo "Usage: $(basename ${0}) mem-trace.mem"
    exit 1
fi

if [[ ! -f "${1}" ]]; then
    echo "File not found: ${1}"; exit 1
    exit 1
fi

cat <<EOM
digraph G {
    fontname="Helvetica"
    graph [nodesep=0.3, ranksep=1.1];

    node[
        shape=doublecircle
        penwidth=2
        style=filled
        color="#CD0000"
        fillcolor="#ECD8DA"
        fontname="Helvetica"
        fontcolor="#CD0000"
    ];
    Head[label="Head"];
    Tail[label="Tail"];

    node[
        shape=plaintext
        color="#28A8E0"
        fontname="Helvetica"
        fontcolor="#1B7095"
        margin=0
    ];

EOM

draw_block() {
    local line="${@}"
    local bgcolor="#CBE1E8"
    local bordercolor="#28A8E0"
    local fontcolor="#1B7095"
    local addresses=$(awk '{print $2}' <<< "${line}")
    local addr1=$(awk -F'-' '{print $1}' <<< "${addresses}")
    local addr2=$(awk -F'-' '{print $2}' <<< "${addresses}")
    local alloc_num=$(sed 's:.*(\([0-9]*\)).*:\1:g' <<< "${line}")
    local bytes=$(awk '{print $4}' <<< "${line}")
    local usage=$(awk '{print $5}' <<< "${line}")
    local alloc_size=$(awk '{print $6}' <<< "${line}")
    local perc=$(awk '{printf("%.0f%%\n", ($1 / $2) * 100.0)}' <<< "${usage} ${bytes}")
    if [[ "${usage}" -eq 0 ]]; then
        perc="free"
        bgcolor="#DDD1E7"
        bordercolor="#652F8E"
        fontcolor="${bordercolor}"
    fi

    local node_name="Alloc_${alloc_num}"
    if [[ -z "${subgraph}" ]]; then
        firsts="${firsts}${node_name}->"
        outer_links="${outer_links}${node_name}; "
    fi
    current_node="${node_name}"
    subgraph="${subgraph}${node_name}->"
cat <<EOM
    ${node_name} [label=<
    <table bgcolor="${bgcolor}" color="${bordercolor}" border="2" cellborder="1" cellspacing="0" cellpadding="3">
        <tr><td><font color="${fontcolor}">Allocation ${alloc_num}</font></td></tr>
        <tr><td align="left"><font color="${fontcolor}">
Size: ${bytes} (${alloc_size})
<br align="left"/>
Usage: ${usage} (${perc})
<br align="left"/>
Start: ${addr1}
<br align="left"/>
End: ${addr2}
<br align="left"/>
</font></td></tr>
</table>>];

EOM
}

draw_region() {
    local line="${@}"

    (( regions++ ))
    subgraph="${subgraph%??}"
    addresses=$(awk '{print $2}' <<< "${line}" | sed 's:-: - :g')
    size=$(awk '{print $3}' <<< "${line}")
cat <<EOM
    subgraph cluster_${regions} {
        label="Mapped Region ${regions}: (${size} bytes)\l${addresses}\l";
        labeljust="l";
        style=filled;
        color="#7F7F7F";
        fontcolor="#7F7F7F"
        fillcolor="#F8F8F8"
        {rank=same; ${subgraph}; }
    };

EOM
    subgraph=""
}

while IFS= read -r line; do
    # Only accept lines starting with REGION/BLOCK
    line_type=$(grep -oiE '^\[REGION\]|\[BLOCK\]' <<< "${line}" \
        | tr 'a-z' 'A-Z' | sed 's:\[::g ; s:\]::g')
    #echo "${line_type}"
    if [[ -z "${line_type}" ]]; then
        continue
    fi
    if [[ "${line_type}" == "BLOCK" ]]; then
        draw_block "${line}"
    elif [[ "${line_type}" == "REGION" ]]; then
        if [[ -n "${current_region}" ]]; then
            draw_region "${current_region}"
            outer_links="${outer_links}${current_node}->"
        fi
        current_region="${line}"
    fi
done < "${1}"

draw_region "${current_region}"

echo "${firsts%??} [weight=10, style=invis];"
echo "${outer_links} ${current_node}->Tail;"

cat <<EOM
}
EOM
