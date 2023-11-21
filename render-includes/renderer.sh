#!/usr/bin/env bash

#      ____                 __          ______     __            
#     / __ \___  ____  ____/ /__  _____/ ____/  __/ /__________ _
#    / /_/ / _ \/ __ \/ __  / _ \/ ___/ __/ | |/_/ __/ ___/ __ `/
#   / _, _/  __/ / / / /_/ /  __/ /  / /____>  </ /_/ /  / /_/ / 
#  /_/ |_|\___/_/ /_/\__,_/\___/_/  /_____/_/|_|\__/_/   \__,_/  
#                                                                

set -euo pipefail

# echo for stderr
function err(){
    echo "[RE]:" "$@" 1>&2
}

EXTRAFLAGS=()
SOURCEFILES=()
CXX="g++"

while [ $# -ne 0 ]
do
    case "$1" in
        -h|--help)
            cat <<EOF
USAGE:
        renderer.sh [-c|--compiler <COMPILER>] [-e|--extra-flag <COMPILER-FLAG>] [-h|--help] <SOURCE-FILES>

DESCRIPTION:
        this is intended for partial preprocessing of source files

        imagine you include both <vector> and "../mymagicalgo.hpp"
        you want to submit your code to a competitive programming site
        they accept single source files :(
        using this you can embed "../mymagicalgo.hpp" without embedding <vector>

        accepts multiple source files in case your build setup is
        to build multiple object files and link them up afterwards

        works well with `#pragma once` directives

        use at your own discretion
        
EXAMPLES:
        renderer.sh main.cpp # uses \`g++\` by default
        renderer.sh --help
        renderer.sh -c "clang++" -e "-I /home/$USER/magiclib" bla.cpp
        renderer.sh -c "clang++ -I /home/$USER/magiclib" bla.cpp # not equivalent to previous line
EOF
            exit 1
            ;;
        -c|--compiler)
            shift
            CXX="$1"
            shift
            ;;
        -e|--extra-flag)
            shift
            EXTRAFLAGS+=("$1")
            shift
            ;;
        *)
            SOURCEFILES+=("$1")
            shift
            ;;
    esac
done

if [ "${#SOURCEFILES[@]}" -eq 0 ]
then
    err "ERROR: missing source file(s)"
    exit 1
fi

err "verifying reasonableness of source files ..."
for FILE in "${SOURCEFILES[@]}"
do
    if ! [ -f "$FILE" ]
    then
        err "ERROR: source file '$FILE' is bad !!!"
        exit 1
    fi
done

err "source files seem regular :)"

err "extracting include dirs ..."
DEFAULTINCLUDEDIRS=($(echo | $CXX -v -E - 2>&1 | sed -n -e '/include </,/End of search list/ p' | head -n -1 | tail -n +2))
err "default include dirs: ${DEFAULTINCLUDEDIRS[@]}"
FULLINCLUDEDIRS=($(echo | $CXX ${EXTRAFLAGS[@]} -v -E - 2>&1 | sed -n -e '/include </,/End of search list/ p' | head -n -1 | tail -n +2))
err "full include dirs: ${FULLINCLUDEDIRS[@]}"

PRAGMAONCEFILES=()

function test_and_set_pragma_once(){
    FILEPATH="$(realpath "$1")"
    for RENDERED in "${PRAGMAONCEFILES[@]}"
    do
        # TODO-think: should this be realpath-ed?
        if [ "$RENDERED" == "$FILEPATH" ]
        then
           return 1
        fi
    done
    PRAGMAONCEFILES+=("$FILEPATH")
    return
}

# ////////
# // $1 //
# ////////
function prettycomment(){
    MID="// $1 //"
    LENGTH=${#MID}
    echo
    head -c $LENGTH /dev/zero | tr '\0' '/'
    printf "\n$MID\n"
    head -c $LENGTH /dev/zero | tr '\0' '/'
    echo
    echo
}

# on success sets FINDINCLUDERESULT + return 0
# on failure return 1
# success means "matches in full, match is not in default"
function find_include_dir_match(){
    FOUNDDIR=
    for DIR in "${FULLINCLUDEDIRS[@]}"
    do
        if [ -f "$DIR/$1" ]
        then
            FOUNDDIR=$DIR
            break
        fi
    done

    if [ "$FOUNDDIR" == "" ]
    then
        return 1
    fi

    for DIR in "${DEFAULTINCLUDEDIRS[@]}"
    do
        if [ "$FOUNDDIR" == "$DIR" ]
        then
            return 1
        fi
    done
    
    FINDINCLUDERESULT="$FOUNDDIR"
    return
}

# TODO-think: does this have to be local? think no, didnt think hard tho
SKIPLINE=
# args: path, name, depth
function render_source_file(){
    err "[$3] rendering $2 ..."
    if [ "$(head -1 "$1")" == "#pragma once" ]
    then
        err "pragma once detected, checking ..."
        if test_and_set_pragma_once "$1"
        then
            err "not rendered, continuing"
            SKIPLINE=x
        else
            err "rendered already, skipping"
            return
        fi
    fi

    prettycomment "RENDER OF $2 [$3]"
    
    while IFS= read -r LINE
    do
        if [ "$SKIPLINE" != "" ]
        then
            SKIPLINE=
            continue
        fi

        REGEX='^#include "(.*)".*$'
        if [[ "$LINE" =~ $REGEX ]]
        then
            render_source_file "$(dirname "$1")/${BASH_REMATCH[1]}" "\"${BASH_REMATCH[1]}\"" $(($3+1))
            continue
        fi

        REGEX='^#include <(.*)>.*$'
        if [[ "$LINE" =~ $REGEX ]]
        then
            if find_include_dir_match "${BASH_REMATCH[1]}"
            then
                render_source_file "$FINDINCLUDERESULT/${BASH_REMATCH[1]}" "<${BASH_REMATCH[1]}>" $(($3+1))
                continue
            fi
        fi

        echo "$LINE"
    done <$1

    prettycomment "FINISH $2 [$3]"
}

prettycomment 'RENDERED WITH `https://github.com/ilazaric/ALL` - `render-includes`'

prettycomment 'ORIGINAL CODE:'

for FILE in "${SOURCEFILES[@]}"
do
    prettycomment "$FILE"
    while IHS= read -r LINE
    do
        echo "// $LINE"
    done <$FILE
done

for FILE in "${SOURCEFILES[@]}"
do
    render_source_file "$FILE" "$FILE" 0
done
