# GAPCM

[[**Homepage**](https://ed7n.github.io/gapcm)]

## Building

    $ make release

### Windows

    $ make mingw-w64

## Formatting

    $ unset files && for file in $(find 'src' -regextype 'egrep' -iregex '.*\.(c|h)'); do files+=("${file}"); done && clang-format -i "${files[@]}"
