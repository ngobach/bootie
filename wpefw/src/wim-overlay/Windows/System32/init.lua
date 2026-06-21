local script_dir = arg[0]:match("^(.*[/\\])")
package.path = script_dir .. "?.lua;" .. package.path
local lib = require("lib")
