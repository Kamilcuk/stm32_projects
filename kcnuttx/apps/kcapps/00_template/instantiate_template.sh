#!/bin/bash
set -euo pipefail

app_dir=${app_dir:-kcapps}
app_name=${app_name:-example}
app_desc=${app_desc:-"This is application ${app_name} compiled from ${app_dir}/${app_name}."}
app_default_priority=${app_default_priority:-100}
app_default_stacksize=${app_default_stacksize:-2048}
outputdir=$(readlink -f ${1:-$(dirname $0)/../${app_name}})

for i in ${!app_*} outputdir; do
	echo "$i=\"${!i}\""
done

read -p "All variables ok? [Y/n]" a
if [ "$a" != Y ]; then exit 1; fi

# some environment
cd $(dirname $(readlink -f $0))
app_desc=$(sed -e 's;/;\\/;g' <<<"$app_desc")
app_DIR=$(tr '[:lower:]' '[:upper:]' <<<"$app_dir")
app_NAME=$(tr '[:lower:]' '[:upper:]' <<<"$app_name")
signstart='\[\['
signstop='\]\]'

# create sedargs
sedargs=()
for i in ${!app_*}; do
	sedargs+=( -e "s/${signstart}${i}${signstop}/${!i}/g" )
done

# actual work
mkdir -p $outputdir
for i in *.tpl; do
	outputfile="$outputdir/$(sed "${sedargs[@]}" -e 's/.tpl$//' <<<"$i")"
	echo "+ tee $outputfile"
	cat $i | sed "${sedargs[@]}" | tee $outputfile
done

for i in *.tpl; do
        outputfile="$outputdir/$(sed "${sedargs[@]}" -e 's/.tpl$//' <<<"$i")"
	echo "+ Written to $outputfile"
done

