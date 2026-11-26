#!/usr/bin/env python3
# time.py [reps] [name=compiler ...] : CPU time (user + system, best of
# reps, default 15) of 50 million iterations of each operation, for each
# compiler given (default: main=../../../atmos) and for lua5.4. The
# variants alternate within each repetition. Prints a Markdown table,
# in seconds. The machine of the paper is noisy (about 10% between
# sessions of measurement, even with the best of 15): compare variants
# only within one run of this script.
import subprocess, os, sys, tempfile
here = os.path.dirname(os.path.abspath(__file__))
args = sys.argv[1:]
reps = int(args.pop(0)) if args and args[0].isdigit() else 15
comps = [a.split('=', 1) for a in args] or [['main', os.path.join(here, '../../../atmos')]]
tmp = tempfile.mkdtemp(prefix='atm-micro-')
ops = subprocess.run([sys.executable, '-I', os.path.join(here, 'gen.py'), tmp, '50000000'],
                     capture_output=True, text=True, check=True).stdout.split()
print('| op | ' + ' | '.join(n for n, _ in comps) + ' | lua54 |')
print('|---' * (len(comps) + 2) + '|')
for k in ops:
    cmds = {}
    for n, c in comps:
        exe = f'{tmp}/{k}-{n}'
        subprocess.run([c, '-o', exe, f'{tmp}/{k}-f.atm'], check=True,
                       env=dict(os.environ, ATMOS_PIN='off', ATMOS_CFLAGS='-O2'))
        cmds[n] = [exe]
    cmds['lua54'] = ['lua5.4', f'{tmp}/{k}.lua']
    best = {}
    for r in range(reps):
        for n, c in cmds.items():
            p = subprocess.Popen(c, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            _, _, ru = os.wait4(p.pid, 0)
            best[n] = min(best.get(n, 9e9), ru.ru_utime + ru.ru_stime)
    print(f'| {k} | ' + ' | '.join('%.3f' % best[n] for n in cmds) + ' |')
    sys.stdout.flush()
