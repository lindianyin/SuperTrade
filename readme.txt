--[[


function testjson()
	print(json.encode({ 1, 2, 'fred', {first='mars',second='venus',third='earth'} }))
end

print("hello world")

local mytest = require("test")
print(mytest.myadd(10,20))

local mysql = require("luamysql")
for k,v in pairs(mysql) do 
	print(k,v)
end
local client = mysql.new_client();

function printTable(tbParam) 
	for k,v in pairs(tbParam) do
		if(type(v) == "table") then
			printTable(tbParam)
		end
		print(k,v)
	end
end

print(type(client))
client:connect({host="183.136.221.50",user="root",passwd="gywl2013",db="booktest",port=3306,unix_socket=0,client_flag=0})
client:ping()
print(client:escape("helloworld>="))
local res = client:execute("SELECT * FROM books;")
--local tbRes = res:fetch_all()


--[==[
		{ "__gc", conn_gc },
		{ "__tostring", conn_tostring },
		{ "set_charset", conn_set_charset },
		{ "set_reconnect", conn_set_reconnect },
		{ "set_connect_timeout", conn_set_connect_timeout },
		{ "set_write_timeout", conn_set_write_timeout },
		{ "set_read_timeout", conn_set_read_timeout },
		{ "set_protocol", conn_set_protocol },
		{ "set_compress", conn_set_compress },
		{ "escape", conn_escape_string },
		{ "connect", conn_connect },
		{ "ping", conn_ping },
		{ "close", conn_close },
		{ "execute", conn_execute },
		{ "commit", conn_commit },
		{ "rollback", conn_rollback },
		{ NULL, NULL },
--]==]


--[===[
while true do 
	local ret = res:fetch()
	if not ret then 
		break
	end
	for k,v in pairs(ret) do 
		print(k,v)
	end
end

client:close()

print(package.cpath)

--]===]