#!/usr/bin/env python3
import os
import zipfile
import json
import time
import threading
from urllib.parse import urlparse, parse_qs
from socketserver import ThreadingMixIn
from http.server import HTTPServer, BaseHTTPRequestHandler
from io import BytesIO

class ThreadingHTTPServer(ThreadingMixIn, HTTPServer):
    daemon_threads = True

PORT = 8080
latest_version = 0
deploy_event = threading.Event()
payload_zip_data = None
current_watch_path = ""
auto_deploy_enabled = False

# Global state for UI
active_clients = {}  # IP -> last_seen_timestamp
logs = []  # List of string logs
lock = threading.Lock()

def add_log(msg):
    with lock:
        timestamp = time.strftime("%H:%M:%S")
        logs.append(f"[{timestamp}] {msg}")
        if len(logs) > 50:
            logs.pop(0)
        print(f"[{timestamp}] {msg}")

def trigger_deploy(path):
    global latest_version, payload_zip_data
    if not os.path.exists(path):
        add_log(f"Error: Deployment target path does not exist: {path}")
        return False
    
    # Create zip package in memory
    zip_buffer = BytesIO()
    file_count = 0
    with zipfile.ZipFile(zip_buffer, 'w', zipfile.ZIP_DEFLATED) as zip_file:
        for root, dirs, files in os.walk(path):
            # Exclude version control or build outputs if applicable
            if '.git' in root or '__pycache__' in root:
                continue
            for file in files:
                if file.startswith('.'):
                    continue
                file_path = os.path.join(root, file)
                rel_path = os.path.relpath(file_path, path)
                zip_file.write(file_path, rel_path)
                file_count += 1
                
    payload_zip_data = zip_buffer.getvalue()
    latest_version += 1
    add_log(f"Successfully packed v{latest_version} from '{path}' ({file_count} files, {len(payload_zip_data)} bytes)")
    
    # Notify all blocking long-poll requests
    deploy_event.set()
    deploy_event.clear()
    return True

# Debounced folder watcher thread
def file_watcher_thread():
    global auto_deploy_enabled, current_watch_path
    last_mtime = 0
    stable_mtime = 0
    stable_checks = 0
    
    while True:
        if auto_deploy_enabled and current_watch_path and os.path.exists(current_watch_path):
            max_mtime = 0
            for root, dirs, files in os.walk(current_watch_path):
                if '.git' in root or '__pycache__' in root:
                    continue
                for f in files:
                    if f.startswith('.'):
                        continue
                    try:
                        mtime = os.path.getmtime(os.path.join(root, f))
                        if mtime > max_mtime:
                            max_mtime = mtime
                    except OSError:
                        pass
            
            if last_mtime == 0:
                last_mtime = max_mtime
            elif max_mtime > last_mtime:
                # Changes detected, debounce to verify write has completed
                if max_mtime == stable_mtime:
                    stable_checks += 1
                else:
                    stable_mtime = max_mtime
                    stable_checks = 0
                
                if stable_checks >= 2:  # Must be stable for ~1 second
                    add_log("[Watcher] Source folder changes stabilized. Auto-deploying...")
                    last_mtime = max_mtime
                    stable_mtime = 0
                    stable_checks = 0
                    trigger_deploy(current_watch_path)
        else:
            stable_checks = 0
            stable_mtime = 0
            
        time.sleep(0.5)

class DeployRequestHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        # Suppress spamming terminal logs for poll endpoints
        return

    def do_GET(self):
        global latest_version, payload_zip_data, current_watch_path, auto_deploy_enabled, active_clients
        
        parsed_url = urlparse(self.path)
        path = parsed_url.path
        
        # Route: Web UI
        if path == "/" or path == "/index.html":
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.end_headers()
            
            # Prune offline clients
            now = time.time()
            client_rows = []
            with lock:
                clients_items = list(active_clients.items())
            for ip, last_seen in clients_items:
                status_class = "status-online" if now - last_seen < 45 else "status-offline"
                status_label = "ONLINE" if now - last_seen < 45 else "OFFLINE"
                last_seen_str = time.strftime("%H:%M:%S", time.localtime(last_seen))
                client_rows.append(f"""
                <div class="client-row">
                    <span class="client-ip">{ip}</span>
                    <span class="client-time">Last poll: {last_seen_str}</span>
                    <span class="status-badge {status_class}">{status_label}</span>
                </div>
                """)
            
            if not client_rows:
                client_rows.append('<div class="no-clients">No WinPE clients have connected yet.</div>')
                
            formatted_logs = "\n".join(reversed(logs))
            
            # HTML with glassmorphism dark-theme style
            html = f"""<!DOCTYPE html>
<html>
<head>
    <title>WinPE Remote Hot-Deploy Panel</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600&family=JetBrains+Mono&display=swap" rel="stylesheet">
    <style>
        :root {{
            --bg-base: #0a0a0c;
            --bg-card: #141419;
            --accent: #00e676;
            --accent-glow: rgba(0, 230, 118, 0.15);
            --text-main: #f0f0f5;
            --text-muted: #8a8a98;
            --border: #2c2c35;
        }}
        
        body {{
            font-family: 'Outfit', sans-serif;
            background: var(--bg-base);
            color: var(--text-main);
            margin: 0;
            padding: 30px;
            display: flex;
            justify-content: center;
        }}
        
        .container {{
            width: 100%;
            max-width: 720px;
            display: flex;
            flex-direction: column;
            gap: 25px;
        }}
        
        .card {{
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 24px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
        }}
        
        .header {{
            display: flex;
            align-items: center;
            justify-content: space-between;
            border-bottom: 1px solid var(--border);
            padding-bottom: 15px;
            margin-bottom: 10px;
        }}
        
        .header h1 {{
            margin: 0;
            font-size: 1.6rem;
            font-weight: 600;
            background: linear-gradient(135deg, #00e676 0%, #00b0ff 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }}
        
        .version-badge {{
            font-family: 'JetBrains Mono', monospace;
            background: #23232a;
            padding: 4px 10px;
            border-radius: 6px;
            font-size: 0.85rem;
            border: 1px solid var(--border);
            color: var(--accent);
        }}
        
        .form-group {{
            display: flex;
            flex-direction: column;
            gap: 8px;
            margin-bottom: 20px;
        }}
        
        label {{
            font-size: 0.9rem;
            font-weight: 600;
            color: var(--text-muted);
        }}
        
        input[type="text"] {{
            background: #0d0d11;
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 12px;
            color: #fff;
            font-size: 0.95rem;
            font-family: 'JetBrains Mono', monospace;
            transition: border-color 0.2s, box-shadow 0.2s;
        }}
        
        input[type="text"]:focus {{
            border-color: var(--accent);
            box-shadow: 0 0 10px var(--accent-glow);
            outline: none;
        }}
        
        .controls-row {{
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-top: 15px;
        }}
        
        .switch-container {{
            display: flex;
            align-items: center;
            gap: 10px;
            cursor: pointer;
        }}
        
        .switch {{
            position: relative;
            display: inline-block;
            width: 44px;
            height: 24px;
        }}
        
        .switch input {{
            opacity: 0;
            width: 0;
            height: 0;
        }}
        
        .slider {{
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #2c2c35;
            transition: .3s;
            border-radius: 24px;
        }}
        
        .slider:before {{
            position: absolute;
            content: "";
            height: 18px;
            width: 18px;
            left: 3px;
            bottom: 3px;
            background-color: white;
            transition: .3s;
            border-radius: 50%;
        }}
        
        input:checked + .slider {{
            background-color: var(--accent);
        }}
        
        input:checked + .slider:before {{
            transform: translateX(20px);
        }}
        
        .btn {{
            background: var(--accent);
            color: #000;
            border: none;
            padding: 12px 24px;
            font-weight: 600;
            border-radius: 8px;
            cursor: pointer;
            font-size: 0.95rem;
            transition: transform 0.1s, opacity 0.2s;
        }}
        
        .btn:hover {{
            opacity: 0.9;
        }}
        
        .btn:active {{
            transform: scale(0.98);
        }}
        
        /* Clients section */
        .client-list {{
            display: flex;
            flex-direction: column;
            gap: 10px;
        }}
        
        .client-row {{
            display: flex;
            align-items: center;
            justify-content: space-between;
            background: #0d0d11;
            padding: 12px 16px;
            border-radius: 8px;
            border: 1px solid var(--border);
        }}
        
        .client-ip {{
            font-family: 'JetBrains Mono', monospace;
            font-weight: 600;
        }}
        
        .client-time {{
            font-size: 0.85rem;
            color: var(--text-muted);
        }}
        
        .status-badge {{
            font-size: 0.75rem;
            font-weight: 600;
            padding: 4px 8px;
            border-radius: 4px;
        }}
        
        .status-online {{
            background: rgba(0, 230, 118, 0.15);
            color: #00e676;
            border: 1px solid rgba(0, 230, 118, 0.3);
        }}
        
        .status-offline {{
            background: rgba(255, 23, 68, 0.15);
            color: #ff1744;
            border: 1px solid rgba(255, 23, 68, 0.3);
        }}
        
        .no-clients {{
            text-align: center;
            padding: 20px;
            color: var(--text-muted);
            font-style: italic;
        }}
        
        /* Log window */
        .console {{
            font-family: 'JetBrains Mono', monospace;
            background: #050508;
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 15px;
            height: 180px;
            overflow-y: auto;
            white-space: pre-wrap;
            font-size: 0.85rem;
            color: #d1d1d6;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <div class="header">
                <h1>WinPE Hot-Deploy</h1>
                <div class="version-badge">Version: v{latest_version}</div>
            </div>
            
            <form action="/deploy" method="POST">
                <div class="form-group">
                    <label>Target Workspace Folder Path (Host)</label>
                    <input type="text" name="path" value="{current_watch_path}" placeholder="/Users/name/Projects/app" required>
                </div>
                
                <div class="controls-row">
                    <label class="switch-container">
                        <span class="switch">
                            <input type="checkbox" name="auto" {"checked" if auto_deploy_enabled else ""} onchange="this.form.submit()">
                            <span class="slider"></span>
                        </span>
                        Auto-deploy on file change
                    </label>
                    
                    <button type="submit" class="btn">Deploy Payload</button>
                </div>
            </form>
        </div>
        
        <div class="card">
            <div class="header" style="border: none; margin: 0; padding: 0 0 10px 0;">
                <h2>WinPE Guest Clients</h2>
            </div>
            <div class="client-list">
                {"".join(client_rows)}
            </div>
        </div>
        
        <div class="card">
            <div class="header" style="border: none; margin: 0; padding: 0 0 10px 0;">
                <h2>Activity Log</h2>
            </div>
            <div class="console">{formatted_logs}</div>
        </div>
    </div>
    
    <script>
        // Auto-refresh the page logs/client list every 2.5 seconds
        setInterval(() => {{
            const watchInput = document.querySelector('input[name="path"]');
            // Only refresh if user is not actively typing in target input
            if (document.activeElement !== watchInput) {{
                fetch(window.location.href)
                    .then(res => res.text())
                    .then(html => {{
                        const parser = new DOMParser();
                        const doc = parser.parseFromString(html, 'text/html');
                        
                        // Update active clients and logs
                        document.querySelector('.client-list').innerHTML = doc.querySelector('.client-list').innerHTML;
                        document.querySelector('.console').innerHTML = doc.querySelector('.console').innerHTML;
                        document.querySelector('.version-badge').innerHTML = doc.querySelector('.version-badge').innerHTML;
                    }})
                    .catch(err => console.log("Refresh error:", err));
            }}
        }}, 2500);
    </script>
</body>
</html>
"""
            self.wfile.write(html.encode("utf-8"))
            return

        # Route: API Poll endpoint
        if path == "/poll":
            client_ip = self.client_address[0]
            with lock:
                active_clients[client_ip] = time.time()
            
            # Extract client version
            query = parse_qs(parsed_url.query)
            client_version = 0
            if "version" in query:
                try:
                    client_version = int(query["version"][0])
                except ValueError:
                    pass
            
            # If server has a newer version, reply immediately
            if latest_version > client_version:
                self.send_json_response({"version": latest_version})
                return
            
            # Otherwise block (long-poll) waiting for a deploy event
            got_signal = deploy_event.wait(timeout=30.0)
            if got_signal and latest_version > client_version:
                self.send_json_response({"version": latest_version})
            else:
                self.send_json_response({"version": client_version, "status": "timeout"})
            return

        # Route: Payload Download endpoint
        if path == "/download":
            if payload_zip_data is None:
                self.send_error(404, "No payload zip available")
                return
            self.send_response(200)
            self.send_header("Content-Type", "application/zip")
            self.send_header("Content-Length", str(len(payload_zip_data)))
            self.end_headers()
            self.wfile.write(payload_zip_data)
            return

        self.send_error(404)

    def do_POST(self):
        global current_watch_path, auto_deploy_enabled
        if self.path == "/deploy":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            params = parse_qs(post_data)
            
            path = params.get("path", [""])[0].strip()
            auto = "auto" in params
            
            current_watch_path = path
            
            # If auto-deploy mode toggled
            if auto != auto_deploy_enabled:
                auto_deploy_enabled = auto
                add_log(f"Auto-Deploy mode set to: {'ENABLED' if auto else 'DISABLED'}")
            
            # If explicit Deploy request (triggered by submit button rather than checkbox change)
            # Checked by verifying if path was submitted in post parameters
            if "path" in params and path:
                add_log(f"Manual deployment triggered for: {path}")
                trigger_deploy(path)
                
            # Redirect back to homepage
            self.send_response(303)
            self.send_header("Location", "/")
            self.end_headers()

    def send_json_response(self, obj):
        data = json.dumps(obj).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

def run():
    add_log("Starting Remote Deploy Server...")
    
    # Start background file watcher
    watcher_thread = threading.Thread(target=file_watcher_thread, daemon=True)
    watcher_thread.start()
    
    server = ThreadingHTTPServer(('0.0.0.0', PORT), DeployRequestHandler)
    add_log(f"Server is listening at http://localhost:{PORT}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    add_log("Server stopped.")

if __name__ == "__main__":
    run()
