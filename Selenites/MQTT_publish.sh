#!/usr/local/bin/Selene
-- Command line tool to publish MQTT data.

local server='localhost'
local port=1883
local topic
local message
local qos = 0;

function help()
	print "Publish a message to a specified MQTT topic"
	print "-h,--host : MQTT server hostname (default localhost)"
	print "-p,--port : MQTT server port number (default 1883)"
	print "-t,--topic : Topic on which to publish"
	print "-m,--message : Message to be published"
	print "-q,--qos : qos to use (default 0)"
	print "-v : be verbose"

	os.exit(10)
end

if not arg then
	help()
end

local i=1
while i<=#arg do
	if arg[i] == '-H' or arg[i] == '--host' then
		i = i + 1
		server = arg[i]
	elseif arg[i] == '-p' or arg[i] == '--port' then
		i = i + 1
		port = tonumber(arg[i])
	elseif arg[i] == '-t' or arg[i] == '--topic' then
		i = i + 1
		topic = arg[i]
	elseif arg[i] == '-m' or arg[i] == '--message' then
		i = i + 1
		message = arg[i]
	elseif arg[i] == '-q' or arg[i] == '--qos' then
		i = i + 1
		qos = tonumber(arg[i])
	elseif arg[i] == '-v' then
		verbose = true
	else
		print("Unknown option '"..arg[i].."'")
		print ""
		help()
	end
	i = i+1
end

if not topic or not message then
	print "Missing topic or message"
	print ""
	help()
end

local url = "tcp://"..server..":"..port
if verbose then
	print("url=", url)
end

local brk,err = SelMQTT.connect( url, { reliable=false, clientID='SelPublish' } )
if not brk then
	print( err )
	return
end
if verbose then
	print "Connected ..."
end

