#!/usr/bin/env python3
"""One JSON-line request per TCP connection to the local runtime debug server."""
import argparse
import json
import socket
from pathlib import Path

def request(command,port=9123):
    with socket.create_connection(('127.0.0.1',port),timeout=10) as sock:
        sock.settimeout(15)
        sock.sendall((json.dumps({'id':1,**command})+'\n').encode())
        line=sock.makefile('r').readline()
        if not line:raise RuntimeError('debug server closed without response')
        return json.loads(line)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('command',help='JSON object, e.g. {"cmd":"frame"}')
    p.add_argument('--port',type=int,default=9123)
    p.add_argument('--output',type=Path)
    a=p.parse_args();result=request(json.loads(a.command),a.port)
    text=json.dumps(result,indent=2)+'\n'
    if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(text)
    print(text,end='')
    if not result.get('ok'):raise SystemExit(1)
