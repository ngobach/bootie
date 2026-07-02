-- remote_deploy_daemon.lua
-- Runs inside WinPE, connects to the host machine (10.0.2.2:8080)
-- Long-polls for updates, downloads zip payloads, extracts them, and runs PECMD LOAD autoexec.ini

local HOST_IP = "10.0.2.2" -- QEMU Host Gateway
local PORT = 8080
local POLL_URL = string.format("http://%s:%d/poll", HOST_IP, PORT)
local DOWNLOAD_URL = string.format("http://%s:%d/download", HOST_IP, PORT)

local current_version = 0
local deployed_versions = {}

-- Load Lua-cURL library
local has_curl, curl = pcall(require, "cURL")
if not has_curl then
  print("[Daemon] Error: Lua-cURL library ('cURL') could not be loaded!")
  print("[Daemon] Available libraries must include 'lcurl' and 'cURL' modules.")
  os.exit(1)
end

-- Helper to sleep using ping (built-in command in WinPE)
local function sleep(sec)
  -- ping -n N+1 waits for N seconds
  os.execute(string.format("ping.exe -n %d 127.0.0.1 >nul", sec + 1))
end

-- Helper to perform HTTP GET request using Lua-cURL
local function http_get(url)
  local response_body = {}
  local c = curl.easy()
  c:setopt_url(url)
  c:setopt_timeout(35) -- Wait longer than Python server's 30s timeout
  c:setopt_writefunction(function(data)
    table.insert(response_body, data)
    return #data
  end)
  
  local ok, err = c:perform()
  c:close()
  
  if ok then
    return table.concat(response_body), nil
  else
    -- Check if it is a timeout (CURLE_OPERATION_TIMEDOUT is code 28)
    local err_code = type(err) == "table" and err.no and err:no()
    if err_code == 28 then
      return nil, "timeout"
    end
    return nil, "error"
  end
end

-- Helper to download binary file using Lua-cURL
local function download_file(url, filepath)
  local file = io.open(filepath, "wb")
  if not file then return false end
  
  local c = curl.easy()
  c:setopt_url(url)
  c:setopt_writefunction(function(data)
    file:write(data)
    return #data
  end)
  
  local ok, err = c:perform()
  c:close()
  file:close()
  
  if not ok then
    print("[Daemon] cURL Download Error: " .. tostring(err))
    os.remove(filepath)
  end
  return ok
end

-- Deploy and extract ZIP payload, then run autoexec.ini via PECMD
local function deploy_payload(zip_path, version)
  local temp_dir = os.getenv("TEMP") or "X:\\Windows\\Temp"
  local extract_path = string.format("%s\\wpefw_app_v%d", temp_dir, version)
  
  -- Create extraction directory
  os.execute(string.format('mkdir "%s" >nul 2>nul', extract_path))
  
  -- Check for 7z.exe or fallback to tar.exe
  local sys_drive = os.getenv("SystemDrive") or "X:"
  local seven_zip = sys_drive .. "\\Program Files\\easy7zip\\7z.exe"
  
  local unzip_cmd
  local f = io.open(seven_zip, "r")
  if f then
    f:close()
    unzip_cmd = string.format('""%s" x "%s" -o"%s" -y >nul"', seven_zip, zip_path, extract_path)
  else
    unzip_cmd = string.format('tar.exe -xf "%s" -C "%s" >nul', zip_path, extract_path)
  end
  
  print(string.format("[Daemon] Extracting v%d to %s", version, extract_path))
  os.execute(unzip_cmd)
  
  -- Run PECMD Load on autoexec.ini inside the extracted directory
  local ini_file = extract_path .. "\\autoexec.ini"
  local pecmd_cmd = string.format('PECMD.EXE LOAD "%s"', ini_file)
  print("[Daemon] Executing payload: " .. pecmd_cmd)
  os.execute(pecmd_cmd)
  
  -- Clean up ZIP
  os.remove(zip_path)
  
  -- Track this version and clean up older versions
  table.insert(deployed_versions, extract_path)
  if #deployed_versions > 2 then
    local old_path = table.remove(deployed_versions, 1)
    print("[Daemon] Cleaning up old directory: " .. old_path)
    os.execute(string.format('rmdir /s /q "%s" >nul 2>nul', old_path))
  end
end

local function main()
  print("[Daemon] Windows PE Remote Deploy Daemon started.")
  print(string.format("[Daemon] Host Endpoint: http://%s:%d", HOST_IP, PORT))
  
  while true do
    local url = string.format("%s?version=%d", POLL_URL, current_version)
    local body, err = http_get(url)
    
    if body then
      -- Parse version from response json (e.g. {"version": 3})
      local next_ver = body:match('"version"%s*:%s*(%d+)')
      next_ver = tonumber(next_ver)
      
      if next_ver and next_ver > current_version then
        print(string.format("[Daemon] Update detected: v%d. Downloading payload...", next_ver))
        
        local temp_dir = os.getenv("TEMP") or "X:\\Windows\\Temp"
        local zip_path = string.format("%s\\payload_v%d.zip", temp_dir, next_ver)
        
        if download_file(DOWNLOAD_URL, zip_path) then
          deploy_payload(zip_path, next_ver)
          current_version = next_ver
        else
          print("[Daemon] Download failed for version " .. next_ver)
        end
      end
    elseif err == "timeout" then
      -- Long poll timed out with no new version; retry immediately
    else
      -- Network error / connection refused. Wait and retry.
      print("[Daemon] Connection failed. Retrying in 2 seconds...")
      sleep(2)
    end
  end
end

-- Prevent GC of the script context while it's active
_G.remote_deploy_daemon_running = true

main()
