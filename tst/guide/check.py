#!/usr/bin/env python3
# Checks that doc/guide.md and tst/guide/ agree: after each marker
# `<!-- tst/guide/NAME.atm -->`, the first fenced block is NAME.atm and
# the second is NAME.out; every NAME.atm has a marker; and the paragraph
# just before a marker ends with a colon (the step player of env-web
# splits the text of the steps by that).
import os, re, sys

here = os.path.dirname(os.path.abspath(__file__))
guide = open(os.path.join(here, '..', '..', 'doc', 'guide.md')).read()

rx = re.compile(
    r'<!-- tst/guide/([0-9a-z-]+)\.atm -->\s*'
    r'```\n(.*?)\n```\s*Output:\s*```\n(.*?)\n```', re.S)

fail = 0
seen = set()
for name, code, out in rx.findall(guide):
    seen.add(name)
    for ext, txt in (('atm', code), ('out', out)):
        want = open(os.path.join(here, name + '.' + ext)).read().rstrip('\n')
        if txt != want:
            print('FAIL: doc/guide.md differs from tst/guide/%s.%s' % (name, ext))
            fail += 1

lines = guide.split('\n')
for i, l in enumerate(lines):
    if l.startswith('<!-- tst/guide/'):
        j = i - 1
        while j >= 0 and lines[j].strip() == '':
            j -= 1
        if not lines[j].rstrip().endswith(':'):
            print('FAIL: the paragraph before %s does not end with a colon' % l)
            fail += 1

for f in sorted(os.listdir(here)):
    if f.endswith('.atm') and f[:-4] not in seen:
        print('FAIL: tst/guide/%s is not in doc/guide.md' % f)
        fail += 1

print('guide: %d examples, %d differences' % (len(seen), fail))
sys.exit(1 if fail else 0)
