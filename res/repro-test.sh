#!/usr/bin/env bash

{
  declare -p ews || declare -A ews=([base]="${0%/*}" [exec]="${0}" \
      [name]='GAPCM Reproducibility Test' [sign]='u0r1 by Brendon, 05/13/2023.')
} &> /dev/null

# Decoder executable.
readonly GAM_DEC='./gamdec'
# Encoder executable.
readonly GAM_ENC='./gamenc'
# Prober executable.
readonly GAM_PRB='./gaminfo'

# For use output parsing.
IFS=$'\n '

GAM.die() {
  (( ${#} )) && echo "${@}" 1>&2
  exit 1
}

type 'cmp' &> /dev/null || GAM.die '`cmp` not found.'
type 'cut' &> /dev/null || GAM.die '`cut` not found.'
type 'find' &> /dev/null || GAM.die '`find` not found.'
type 'sort' &> /dev/null || GAM.die '`sort` not found.'
type "${GAM_DEC}" &> /dev/null || GAM.die '`'"${GAM_DEC}"'` not found.'
type "${GAM_ENC}" &> /dev/null || GAM.die '`'"${GAM_ENC}"'` not found.'
type "${GAM_PRB}" &> /dev/null || GAM.die '`'"${GAM_PRB}"'` not found.'
(( ${#} )) || GAM.die 'No file.'
echo -e "${ews[name]}"' '"${ews[sign]}"'\n\nWorking directory:\n  '"$(pwd)"'
Input directory:\n  '"${1}"
[ -e "${1}" ] || GAM.die 'Not found.'
[ -d "${1}" ] || GAM.die 'Not a directory.'
[ -r "${1}" ] || GAM.die 'No read permission.'
[ -x "${1}" ] || GAM.die 'No execute permission.'
echo 'Reading.'
gamItms="$(find "${1}" -maxdepth 1 -type f -iname '*.pcm' -printf '%p'"${IFS}" \
    | sort)"
echo 'Now testing.'
for gamItm in ${gamItms}; do
  echo -n '  '"${gamItm}"
  for gamFld in $("${GAM_PRB}" "${gamItm}" | cut -d ':' -f 2 -s); do
    gamFlds+=("${gamFld}")
  done
  {
    "${GAM_DEC}" -p 0 -l 1 -t -o - "${gamItm}" \
        | "${GAM_ENC}" -c "${gamFlds[0]}" -m "${gamFlds[1]}" -n \
            "${gamFlds[2]}" -ea '0x'"${gamFlds[3]}" '0x'"${gamFlds[4]}" \
            '0x'"${gamFlds[5]}" '0x'"${gamFlds[6]}" '0x'"${gamFlds[7]}" \
            '0x'"${gamFlds[8]}" -ep "${gamFlds[9]}" -ed "${gamFlds[10]}" -el \
            "${gamFlds[11]}" "${gamFlds[12]}" "${gamFlds[13]}" -p \
            "${gamFlds[14]}" -t -o - - \
        | cmp -s "${gamItm}" -
  } 2> /dev/null && {
    echo -en '\r  '"${gamItm//?/ }"'\r' || :
  } || GAM.die '  Mismatch.'
  unset gamFlds[@]
done
echo 'Done.'
