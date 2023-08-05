local opt = {}

opt.staticComp = false
opt.engineShared = true
opt.linuxWayland = true
opt.linuxX11 = true

if BuildConf.TargetPlatform == Platform.Windows then
    opt.staticComp = true
    log.info("On Windows, using static compile")
end

return opt
