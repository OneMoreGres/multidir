#!/bin/bash

set -e

if [ $# == 0 ]; then echo "Usage: $0 <tag_name>."; exit 0; fi

SELF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

tag=$1
release=`curl -s https://api.github.com/repos/OneMoreGres/multidir/releases/tags/$tag`

export PYTHONIOENCODING=utf8    
release_id=`echo "$release" | python -c "import sys, json; print(json.load(sys.stdin)['id'])"`

if [ -z "$release_id" ]; then echo "No release for tag"; exit 1; fi
echo "Editing release $release_id"

changes=`$SELF_PATH/rescent_changes.sh $SELF_PATH/Changelog_en.txt`
if [ -z "$changes" ]; then echo "No changes found"; exit 1; fi
echo "$changes"

python -c "import sys, json; print(json.dumps({'body':'''$changes'''}))" > changelog

curl -X PATCH -H "Authorization: token $GH_OAUTH" -d @changelog https://api.github.com/repos/OneMoreGres/multidir/releases/$release_id
