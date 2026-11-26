# Converts a copy of sdl-pingus and env-sdl to `import` (an experiment).
import re,sys,glob,os
root=sys.argv[1]
def split(lines, lo, hi):
    once=[]
    for i in range(lo,hi):
        l=lines[i]
        if not l.startswith('static') or '(' in l.split('//')[0]: continue
        m=re.match(r'^static\s+(.*?)\s*(=\s*[^;]*)?;(.*)$',l)
        if not m: continue
        decl,init,rest=m.group(1),m.group(2) or '',m.group(3)
        once.append((decl+' '+init).rstrip()+';')
        lines[i]=f'extern {decl};{rest}'
    return once
for f in glob.glob(root+'/sdl-pingus/**/*.atm',recursive=True):
    if '/data/' in f: continue
    s=open(f).read(); t=re.sub(r'(?m)^include "','import "',s)
    if s!=t: open(f,'w').write(t)
p=root+'/env-sdl/sdl.atm'; L=open(p).read().split('\n')
s=next(i for i,l in enumerate(L) if l.startswith('```:prelude'))
e=next(i for i in range(s+1,len(L)) if L[i].startswith('```'))
once=split(L,s,e)
a=next(i for i in range(s,e) if L[i].startswith('int main'))
b=a
while L[b]!='}': b+=1
main=L[a:b+1]; L[a:b+1]=[]
e=next(i for i in range(s+1,len(L)) if L[i].startswith('```'))
L[e+1:e+1]=['```:prelude.c']+once+['']+main+['```']
open(p,'w').write('\n'.join(L))
p=root+'/sdl-pingus/engine/pingus.h'; H=open(p).read().split('\n')
once=split(H,0,len(H)); open(p,'w').write('\n'.join(H))
p=root+'/sdl-pingus/engine/sdl.atm'; L=open(p).read().split('\n')
s=next(i for i,l in enumerate(L) if l.startswith('```:prelude'))
e=next(i for i in range(s+1,len(L)) if L[i].startswith('```'))
L[e+1:e+1]=['```:prelude.c']+once+['```']
open(p,'w').write('\n'.join(L))

# the tags: what was a prelude is a header
for f in glob.glob(root+'/**/*.atm',recursive=True):
    if '/data/' in f: continue
    s=open(f).read(); t=re.sub(r'(?m)^```:prelude$','```:prelude.h',s)
    if s!=t: open(f,'w').write(t)
