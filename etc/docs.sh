#!/bin/sh
# Tracking of the documentation that the author edits by hand
# (session atmos-docs, atmos-docs#2).
#
#   etc/docs.sh status [doc...]       what is new on main for each document
#   etc/docs.sh meld <doc> [mine]     the files for meld, then meld itself
#   etc/docs.sh files <doc>           only write the files and print their paths
#
# Each document has a reference (the original version) and a base: the
# last commit of main whose version of the document the author has taken
# into his merge. The base is the newest of:
#   - the table "Bases" of .claude/plans/TODO-docs.md;
#   - a trailer "Docs-Base: <doc> <sha>" in a commit of his.
# So `git diff <base>..main -- <doc>` is exactly what is new for him.
#
# Options (environment): DOCS_MAIN (default origin/main, fetched first;
# DOCS_NOFETCH=1 skips the fetch), DOCS_OUT (default $TMPDIR/atmos-docs).

set -e
cd "$(git rev-parse --show-toplevel)"

TABLE=.claude/plans/TODO-docs.md
MAIN=${DOCS_MAIN:-origin/main}
OUT=${DOCS_OUT:-${TMPDIR:-/tmp}/atmos-docs}
DOCS="doc/manual.md doc/guide.md README.md"

die () { echo "docs.sh: $*" >&2; exit 1; }

fetch () {
    [ -n "$DOCS_NOFETCH" ] && return 0
    case "$MAIN" in origin/*)
        git fetch -q origin "${MAIN#origin/}" 2>/dev/null \
            || echo "docs.sh: no fetch, using $MAIN as it is" >&2 ;;
    esac
}

# A column of the table "Bases": | doc | base | reference | pass |
# ($2 doc, $3 base, $4 reference, $5 pass: "first" until his first merge)
row () {
    awk -v d="$1" -F'|' '
        /^## Bases/ { on=1; next }  /^## / { on=0 }
        on { gsub(/[ `]/, "", $2); if ($2 == d) { gsub(/[ `]/, "", $'"$2"'); print $'"$2"'; exit } }
    ' "$TABLE"
}

# The author's commits are those not written by an automatic session.
own_last () {
    git log "$MAIN" --format='%H %an' -- "$1" | awk '$2 != "Claude" { print $1; exit }'
}

trailer_base () {
    git log "$MAIN" --format='%H%n%(trailers:key=Docs-Base,valueonly)' \
        | awk -v d="$1" 'NF == 2 && $1 == d { print $2; exit }'
}

base_of () {
    t=$(row "$1" 3); [ -n "$t" ] || die "no base for $1 in $TABLE"
    t=$(git rev-parse --verify -q "$t^{commit}") || die "bad base for $1"
    c=$(trailer_base "$1")
    if [ -n "$c" ] && c=$(git rev-parse --verify -q "$c^{commit}") \
       && git merge-base --is-ancestor "$t" "$c"; then t=$c; fi
    echo "$t"
}

# A reference is <sha> (this repository) or <url>@<sha>[:path].
show_ref () {
    r=$(row "$1" 4)
    case "$r" in
        *@*) url=${r%@*}; rest=${r##*@}; sha=${rest%%:*}
             path=$1; case "$rest" in *:*) path=${rest#*:} ;; esac
             git cat-file -e "$sha^{commit}" 2>/dev/null \
                 || git fetch -q "$url" "$sha" || die "cannot fetch $url@$sha"
             git show "$sha:$path" ;;
        *)   git show "$r:$1" ;;
    esac
}

count () { git rev-list --count --no-merges "$@" -- "$doc"; }

status () {
    [ $# -gt 0 ] || set -- $DOCS
    main=$(git rev-parse --short "$MAIN")
    echo "main: $main"
    for doc; do
        base=$(base_of "$doc"); own=$(own_last "$doc")
        echo
        o=none; [ -n "$own" ] && o=$(git rev-parse --short "$own")
        echo "== $doc   base $(git rev-parse --short "$base")   his last commit $o"
        n=$(count "$base..$MAIN")
        st=$(git diff --shortstat "$base" "$MAIN" -- "$doc" | sed 's/^ *1 file changed, //')
        echo "   new since base: $n commits${st:+ ($st)}"
        git log --no-merges --format='   %h %ad %s' --date=short "$base..$MAIN" -- "$doc"
        if [ -n "$own" ] && ! git merge-base --is-ancestor "$own" "$base"; then
            echo "   new since his last commit: $(count "$own..$MAIN") commits"
        fi
    done
}

files () {
    doc=$1; [ -n "$doc" ] || die "which document?"
    name=$(basename "$doc" .md)
    mkdir -p "$OUT"
    base=$(base_of "$doc")
    show_ref "$doc"        > "$OUT/$name.ref.md"
    git show "$base:$doc"  > "$OUT/$name.base.md"
    git show "$MAIN:$doc"  > "$OUT/$name.main.md"
    echo "$OUT/$name.ref.md   reference"
    echo "$OUT/$name.base.md  base $(git rev-parse --short "$base")"
    echo "$OUT/$name.main.md  main $(git rev-parse --short "$MAIN")"
}

cmd=${1:-status}; [ $# -gt 0 ] && shift
case "$cmd" in
    status) fetch; status "$@" ;;
    files)  fetch; files "$1" ;;
    meld)   fetch; files "$1"
            doc=$1; name=$(basename "$doc" .md); mine=${2:-$doc}
            if [ "$(row "$doc" 5)" = first ]; then
                # first pass: the reference, his version, the automatic one
                exec meld "$OUT/$name.ref.md" "$mine" "$OUT/$name.main.md"
            else
                # later passes: only what changed from the base to main
                exec meld "$OUT/$name.base.md" "$mine" "$OUT/$name.main.md"
            fi ;;
    *) die "unknown command $cmd (status, meld, files)" ;;
esac
