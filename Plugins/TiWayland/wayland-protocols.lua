protocol_dir = ""
--if RunCmd returns something other than 0, something messed up
exit_code, output, error = util.runCmd("pkg-config", "--variable=pkgdatadir wayland-protocols")
if exit_code == 0 then
    protocol_dir = output
    --see if protocol_dir starts with a double slash
    if string.sub(protocol_dir, 1, 2) == "//" then
    	--remove the double slash
        protocol_dir = string.sub(protocol_dir, 2)
    end
else
    log.error("Returned: " .. error)
    log.fatal("pkg-config failed to find wayland-protocols")
    return nil
end

local proto_dir = nil

if borzwlproto == nil then
    project("borzwlproto", Language.C, BinType.StaticLib)
    proto_dir = path.combine(borzwlproto.IntermediateDirectory, "protocols")
    dir.create(proto_dir)
    borzwlproto.AddIncludePath(proto_dir, true)
end

if proto_dir == nil then
    proto_dir = path.combine(borzwlproto.IntermediateDirectory, "protocols")
end

return function(protocols)
    for i = 1, #protocols do
    	proto_loc = path.combine(protocol_dir, unpack(protocols[i]))
    	if file.exists(proto_loc) != true then
    		log.fatal("Failed to find protocol: " .. proto_loc)
    	end

    	file_name = path.getFileNameNoExt(proto_loc)
    	local header_file_name = file_name .. "-client-protocol.h"
    	local source_file_name = file_name .. "-client-protocol.c"

    	local header_path = path.combine(proto_dir, header_file_name)
    	local src_path = path.combine(proto_dir, source_file_name)

    	if not file.exists(header_path) then
    		exit_code, output, error = util.runCmd("wayland-scanner", "client-header " .. proto_loc .. " " .. header_path)
    	end

    	if not file.exists(src_path) then
    		exit_code, output, error = util.runCmd("wayland-scanner", "private-code " .. proto_loc .. " " .. src_path)
        end
        borzwlproto.AddSourceFile(src_path)
    end
    return borzwlproto
end