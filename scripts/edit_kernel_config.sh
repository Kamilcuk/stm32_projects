#!/bin/bash
set -euo pipefail
if ${DEBUG:-false}; then set -x; fi
# functions #####################################

usage() {
	cat <<EOF
USAGE: 
	$0 [options] <config string> [<value to set>]
	$0 [options] clear <config string>

Modifies input stream of .config file.

Options:
	-c <.config file>	- read/modify this config file
	-h 			- show this text and exit;

Written by Kamil Cukrowski. Version 0.0.1
Jointly under MIT License and Beerware License.
EOF
}


config_get() { local key=$1       ; shift  ; sed -n -e '/^[# ]\?'"$key"'[ =]/p'              "$@"; }
config_set() { local key=$1 val=$2; shift 2; sed    -e '/^[# ]\?'"$key"'[ =]/s/.*/'"$val"'/' "$@"; }
config_set_safe() {
        local file="$1" key="$2" val="$3"
        if [ "$file" != "-" ]; then
                local tmp;
                tmp=$(config_get "$key" <"$file")
                if [ "$tmp" != "$val" ]; then
                        config_set "$key" "$val" -i "$file"
                fi
        else
                config_set "$key" "$val"
        fi
}

runtests() {
	local config
	config="# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
CONFIG_VAR1=0
CONFIG_VAR2=1
# CONFIG_VAR3 is not set"
	tests=(
		"cat" "$config"
		"$0 ULUMULU" ""
		"$0 CONFIG_VAR1" "CONFIG_VAR1=0"
		"$0 clear CONFIG_VAR1" "# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
# CONFIG_VAR1 is not set
CONFIG_VAR2=1
# CONFIG_VAR3 is not set"
		"$0 clear CONFIG_VAR2" "# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
CONFIG_VAR1=0
# CONFIG_VAR2 is not set
# CONFIG_VAR3 is not set"
		"$0 clear CONFIG_VAR3" "$config"
		"$0 CONFIG_VAR2" "CONFIG_VAR2=1"
		"$0 CONFIG_VAR1 10" "# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
CONFIG_VAR1=10
CONFIG_VAR2=1
# CONFIG_VAR3 is not set"
	)
	for ((i=0;i<${#tests[@]};i+=2)); do
 		test=${tests[$i]}
 		exp_result=${tests[$((i+1))]}

		echo "+ $test <<<\"\$config\""
		result=$($test <<<"$config" | tee /dev/stderr)

		if [ "$exp_result" != "$result" ]; then
			echo "ERROR test=\"$test\" exp_result=\"$exp_result\""
			exit 1
		fi
	done

	tests=(
		"$0 clear CONFIG_VAR1" "# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
# CONFIG_VAR1 is not set
CONFIG_VAR2=1
# CONFIG_VAR3 is not set"
		"$0 clear CONFIG_VAR2" "# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
CONFIG_VAR1=0
# CONFIG_VAR2 is not set
# CONFIG_VAR3 is not set"
		"$0 clear CONFIG_VAR3" "$config"
		"$0 CONFIG_VAR1 10" "# CONFIG_VAR12 is not set
CONFIG_VAR=dont touch
CONFIG_VAR1=10
CONFIG_VAR2=1
# CONFIG_VAR3 is not set"
	)

	local tmpfile
	tmpfile=$(mktemp)
	for ((i=0;i<${#tests[@]};i+=2)); do
 		test=${tests[$i]}
 		exp_result=${tests[$((i+1))]}

		echo "+ $test -c \"$tmpfile\""
		echo "$config" > $tmpfile
		$test -c "$tmpfile"
		result=$(cat "$tmpfile" | tee /dev/stderr)

		if [ "$exp_result" != "$result" ]; then
			echo "ERROR test=\"$test\" exp_result=\"$exp_result\""
			exit 1
		fi
	done
	rm $tmpfile
	echo ----===== SUCCESS =====-----
}


# main #########################################

if [ $# -lt 1 ]; then usage; exit 1; fi
ARGS=$(getopt -n $(basename $0) -o hc: -- "$@")
eval set -- "$ARGS"
configfile="-"
while true; do
	case "$1" in
		-c) configfile="$2"; shift; ;;
		-h) usage; exit 0; ;;
		--) shift; break; ;;
		*) echo "Internal error"; exit 1; ;;
	esac
	shift
done
if [ $# -lt 1 ]; then usage; exit 1; fi

case "$1" in
test)
	runtests
	;;
clear)
	config_set_safe "${configfile}" "$2" "# $2 is not set"
	;;
*)
	if [ $# -eq 2 ]; then
		config_set_safe "${configfile}" "$1" "$1=$2"
	else
		cat "$configfile" | config_get "$1"
	fi
	;;
esac

