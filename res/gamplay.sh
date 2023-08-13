#!/usr/bin/env bash

# GAMplay u0r1 by Brendon, 08/12/2023.
# ——GAPCM playback helper. https://ed7n.github.io/gapcm

# Decoder executable.
readonly GAM_DEC='./gamdec'
# Fade in duration in seconds.
readonly GAM_FID='0.0795'
# Fade out duration in seconds.
readonly GAM_FOD=10
# Loop count excluding fade out.
readonly GAM_LOC=6
# Loop activation threshold in seconds.
readonly GAM_LOT=7
# Low-pass filter cutoff for 16276 Hz in Hz.
readonly GAM_LPC=5000
# Prober executable.
readonly GAM_PRB='./gaminfo'
# Sample rate in Hz.
readonly GAM_RAT=16276

# For use with header output parsing.
IFS=$'\n '

GAM.die() {
  (( ${#} )) && echo -e "${@}" 1>&2
  exit 1
}

type 'cut' &> /dev/null || GAM.die '`cut` not found.'
type 'ffplay' &> /dev/null || GAM.die '`ffplay` not found.'
type "${GAM_DEC}" &> /dev/null || GAM.die '`'"${GAM_DEC}"'` not found.'
type "${GAM_PRB}" &> /dev/null || GAM.die '`'"${GAM_PRB}"'` not found.'
(( ${#} )) || GAM.die 'No file.'
[ -e "${1}" ] || GAM.die 'Not found.'
[ -r "${1}" ] || GAM.die 'No read permission.'
case $(( ($("${GAM_PRB}" -bf) + 1) % 2 )) in
  0 )
    gamFmt='u16le' ;;
  1 )
    gamFmt='u8' ;;
  * )
    GAM.die 'Bad build flags.' ;;
esac
for gamFld in $("${GAM_PRB}" "${1}" | cut -d ':' -f 2 -s); do
  gamFlds+=("${gamFld}")
done
gamCnl="${gamFlds[0]}"
(( gamMrk = gamFlds[1] * (1024 / gamCnl) ))
(( gamLen = gamFlds[2] -  gamMrk ))
gamRat="${GAM_RAT}"
(( gamCnl == 2 )) && {
  gamCnm='0|1'
  (( GAM_RAT > 16274 && GAM_RAT < 16300 )) && {
    (( GAM_RAT < 16287 )) && {
      gamRat=16274 || :
    } || gamRat=16300
    echo 'Set sample rate to '"${gamRat}"'.'
  } || :
} || gamCnm='0'
gamAfc='afade=t=in:d='"${GAM_FID}"','
(( gamLen / GAM_LOT < GAM_RAT - 1 )) && {
  gamLoc=1
  gamOpr='single' || :
} || {
  (( gamLoc = GAM_LOC + GAM_FOD / GAM_LOT + 1 ))
  gamEnd="${gamLen}"' * '"${GAM_LOC}"' + '"${gamMrk}"
  gamAfc+='afade=t=out:start_sample='"${gamEnd}"':d='"${GAM_FOD}"','
  gamAfc+='atrim=end_pts='"${gamEnd}"' + '"${gamRat}"' * '"${GAM_FOD}"','
  gamOpr='loop'
}
gamLpc="${GAM_LPC}"' / 16276 * '"${GAM_RAT}"
gamAfc+='aresample='"${gamRat}"' * 2:filter_size=0:phase_shift=0,'
gamAfc+='channelmap='"${gamCnm}"','
gamAfc+='lowpass='"${gamLpc}"':p=1,lowpass='"${gamLpc}"':p=1,'
echo 'Now playing in '"${gamOpr}"' mode.'
"${GAM_DEC}" -p 0 -l "${gamLoc}" -o '-' "${1}" | ffplay -autoexit -loglevel \
    'warning' -f "${gamFmt}" -ac "${gamCnl}" -ar "${gamRat}" -af \
    "${gamAfc:0:-1}" '-' && echo 'Done.'
