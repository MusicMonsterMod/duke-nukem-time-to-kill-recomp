#!/usr/bin/env python3
"""Summarize D07 launch/flight/impact observations, not synthetic hit claims."""
import argparse
from collections import Counter
import json
import math
from pathlib import Path
import re


def vector(text): return [float(v) for v in text.split(',')]

def summarize(directory):
    report=json.loads((directory/'report.json').read_text())
    shots=[];launches=[];flights=[];impacts=[];pending=[];tracked={};queries=[];current=None
    for line in (directory/'runtime.log').read_text().splitlines():
        if not line.startswith('ttk-'): continue
        fields=dict(re.findall(r'(\w+)=([^ ]+)',line))
        if line.startswith('ttk-aim-query'):
            queries.append(int(fields['kind']))
        elif line.startswith('ttk-aim-shot'):
            current=dict(weapon=int(fields['weapon']),target=vector(fields['target']),direction=vector(fields['direction']),cover=int(fields['cover']),camera_query_kind=queries[-2] if len(queries)>=2 else None)
            shots.append(current)
        elif line.startswith('ttk-launch'):
            matched=current if current and {4:0,5:1,6:6,7:7,8:8,11:13,12:4,13:11}.get(current['weapon'])==int(fields['type']) else None
            row=dict(type=int(fields['type']),origin=vector(fields['origin']),direction=vector(fields['direction']),aim=matched)
            launches.append(row);pending.append(row)
        elif line.startswith('ttk-flight'):
            row=dict(projectile=fields['projectile'],type=int(fields['type']),origin=vector(fields['from']),end=vector(fields['to']),direction=vector(fields['direction']))
            for candidate in pending:
                if candidate['type']==row['type'] and candidate['origin']==row['origin'] and candidate['direction']==row['direction']:
                    tracked[row['projectile']]=candidate;pending.remove(candidate);break
            flights.append(row)
        elif line.startswith('ttk-impact'):
            launch=tracked.pop(fields['projectile'],None)
            row=dict(projectile=fields['projectile'],type=int(fields['type']),point=vector(fields['point']))
            if launch:
                row['launch']=launch
                delta=[row['point'][i]-launch['origin'][i] for i in range(3)]
                d=launch['direction'];length=math.sqrt(sum(v*v for v in d));along=sum(delta[i]*d[i]/length for i in range(3))
                row['distance_along_launch']=along
                row['perpendicular_launch_error']=math.sqrt(sum((delta[i]-along*d[i]/length)**2 for i in range(3)))
                if launch['aim']:
                    row['distance_to_view_target']=math.dist(row['point'],launch['aim']['target'])
            impacts.append(row)
    samples=[]
    for r in report.get('weapon_research',[]):
        samples.append(dict(name=r['name'],weapon_slot=r['weapon_slot'],ammo=r['ammo'][r['weapon_slot']],aim=r['input']['controls']['aim'],player_position=r['state']['player_position_candidate'],animation=r['state']['player_animation']))
    return dict(run=str(directory),binary_sha256=report['binary_sha256'],exit_code=report.get('exit_code'),error=report.get('error'),samples=samples,shots=shots,launch_types=dict(Counter(x['type'] for x in launches)),flight_types=dict(Counter(x['type'] for x in flights)),impacts=impacts,fixture_writes=report.get('fixture_writes',[]),weapon_selection=report.get('weapon_selection',[]),limits='Impact records are original collision-consumer calls. Distance to target may differ due to intervening cover, moving actors, spread, gravity or bounce. Synthetic selection does not establish natural acquisition, animation or models.')

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('directory',type=Path);parser.add_argument('--output',required=True,type=Path)
    a=parser.parse_args();a.output.write_text(json.dumps(summarize(a.directory),indent=2)+'\n')
