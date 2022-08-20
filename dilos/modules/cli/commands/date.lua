local cmd = {}

cmd.desc = [[print the system date and time]]
cmd.help = [[Usage: date

Print OSv reported date.]]

cmd.main = function()
  local content, status = osv_request({"os", "date"}, "GET")
  osv_resp_assert(status, 200)
  io.write(content, '\n')
end

return cmd
