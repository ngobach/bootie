local ffi = require("ffi")
ffi.cdef[[
  int MessageBoxA(void *hWnd, const char *lpText, const char *lpCaption, unsigned int uType);
]]

local M = {}

function M.MessageBox(text, caption, uType)
  ffi.C.MessageBoxA(nil, text, caption or "Message", uType or 0)
end

return M
