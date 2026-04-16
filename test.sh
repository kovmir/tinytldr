#!/usr/bin/env bash
# Unit tests for tinytldr
# Usage: see Makefile

set -e

_url=$1
_dir=$2
_path=$3
_lang=$4

_test_dir='common'
_test_page='testpage1.md'
# Simulate pages archive.
mkdir -p "$_dir/$_lang/$_test_dir"
cat <<EOF > "$_dir/$_lang/$_test_dir/$_test_page"
# heading

> subheading

- cmd description:

\`cmd itself\`
EOF
zip -r "$(basename "$_url")" "$_dir"

_port=$(echo "${_url#*://}" | cut -d'/' -f1 | cut -d':' -f2)
python -m http.server "$_port" &> /dev/null &

./tldr -u

if [[ -f "$_path/index" ]]; then
	echo '1 OK'
else
	echo '1 NOT OK'
	exit 1
fi

_expected="$_test_dir/$_test_page"
if cmp -s "$_path/index" <(echo "$_expected"); then
	echo '2 OK'
else
	echo '2 NOT OK'
	exit 1
fi

if [[ -f "$_path/$_lang/$_test_dir/$_test_page" ]]; then
	echo "3 OK"
else
	echo "3 NOT OK"
	exit 1
fi

_expected=$'^# heading\n-^> subheading\n-^- cmd description:\n-^`cmd itself`\n-'
_actual="$(./tldr testpage1)"
if [[ "$_actual" == "$_expected" ]]; then
	echo "4 OK"
else
	echo "4 NOT OK"
	exit 1
fi

rm -rf "$_dir" "$_path" "$(basename "$_url")"
