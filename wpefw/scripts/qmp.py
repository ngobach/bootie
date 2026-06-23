#!/usr/bin/env python3
import json, socket, sys

def recvall(sock):
    buf = b''
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        buf += chunk
        try:
            return json.loads(buf)
        except json.JSONDecodeError:
            continue

if len(sys.argv) < 2:
    print("usage: qmp.py <json-command>", file=sys.stderr)
    sys.exit(1)

sock = socket.socket(socket.AF_UNIX)
sock.settimeout(5)
sock.connect('/tmp/qmp.sock')
recvall(sock)
sock.send(json.dumps({'execute': 'qmp_capabilities'}).encode() + b'\n')
recvall(sock)
sock.send(json.dumps(json.loads(sys.argv[1])).encode() + b'\n')
resp = recvall(sock)
sock.close()

if 'error' in resp:
    print(json.dumps(resp['error']), file=sys.stderr)
    sys.exit(1)
else:
    print(json.dumps(resp, indent=2))
