#!/usr/bin/env python3
"""LOVE EQ mobile icon: opaque 1024x1024 RGB PNG; no external dependencies."""
import math, struct, zlib, pathlib, sys
N=1024
def chunk(kind, data):
    return struct.pack(">I",len(data))+kind+data+struct.pack(">I",zlib.crc32(kind+data)&0xffffffff)
def smooth(a,b,t):
    return tuple(round(a[i]+(b[i]-a[i])*max(0.,min(1.,t))) for i in range(3))
def clamp(x):
    return max(0.,min(1.,x))
nodes=[(-.64,.13),(-.29,-.22),(0.,.02),(.29,.28),(.64,-.12)]
rows=bytearray()
for j in range(N):
    y=(j+.5-N/2)/(N/2)
    rows.append(0)
    for i in range(N):
        x=(i+.5-N/2)/(N/2)
        r=math.hypot(x,y)
        c=smooth((38,50,63),(10,16,25),clamp(r*.82+.14*y))
        if abs(x)<.78 and abs(y)<.78:
            grid=max(0.,1.-min(abs(x-round(x/.19)*.19),abs(y-round(y/.19)*.19))*240)
            c=smooth(c,(72,86,97),grid*.21)
        # Metallic circular instrument bezel.
        c=smooth(c,(165,139,95),clamp(1.-abs(r-.79)*300)*.68)
        c=smooth(c,(80,92,103),clamp(1.-abs(r-.755)*330)*.52)
        # Continuous equalizer curve; cubic interpolation through the nodes.
        if -.64<=x<=.64:
            for k in range(4):
                x0,y0=nodes[k]; x1,y1=nodes[k+1]
                if x<=x1:
                    t=(x-x0)/(x1-x0)
                    cy=y0+(y1-y0)*t*t*(3-2*t)
                    dist=abs(y-cy)
                    c=smooth(c,(245,178,80),clamp(1-dist*26)*.18)
                    c=smooth(c,(244,211,149),clamp((.013-dist)*650))
                    break
        for x0,y0 in nodes:
            rr=math.hypot(x-x0,y-y0)
            c=smooth(c,(252,218,151),clamp((.027-rr)*650))
            c=smooth(c,(22,31,42),clamp((.012-rr)*650))
        # A compact red heart above the curve, as the LOVE EQ mark.
        hx=x/.24; hy=-(y+.36)/.24
        if abs(hx)<1.5 and abs(hy)<1.5:
            h=(hx*hx+hy*hy-1)**3-hx*hx*hy**3
            a=clamp(-h*35)
            c=smooth(c,smooth((244,130,115),(164,38,53),clamp((y+.62)*2.)),a)
        rows.extend(c)
out=pathlib.Path(sys.argv[1]);out.parent.mkdir(parents=True,exist_ok=True)
out.write_bytes(b"\x89PNG\r\n\x1a\n"+chunk(b"IHDR",struct.pack(">IIBBBBB",N,N,8,2,0,0,0))+chunk(b"IDAT",zlib.compress(rows,9))+chunk(b"IEND",b""))
print("LOVE EQ icon:",out,"1024x1024 RGB")
