local opt = {}

opt.staticComp = false

if BuildConf.TargetPlatform == Platform.Windows then
    opt.staticComp = true
    log.info("On Windows, using static compile")
end


return opt
