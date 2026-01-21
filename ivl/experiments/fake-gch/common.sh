# DO NOT EXECUTE THIS
# other executable scripts source this

set -euo pipefail
set -x

OBJDIR="${0%.*}-objdir"

if [ -d "$OBJDIR" ]
then
    rm -rf "$OBJDIR"
fi

mkdir "$OBJDIR"
cp heavy-header.hpp main1.cpp "$OBJDIR/"
cd "$OBJDIR"
