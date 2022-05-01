# VFPC Plugin
 
The HK-VFPC (Hong Kong Virtual Flight Plan Checker) is a plugin for EuroScope for HKvACC controllers to check flight plans against relevant route and altitude restrictions. 
Originally developed by hpeter2 and DrFreas, it has been expanded on with modified functionality to better suit the needs of HKvACC. You can find the codebase of the original plugin [here](https://github.com/hpeter2/VFPC).
As such, the code will continue to remain open source and free to download for any controllers. However, other vACCs looking to implement or modify a similar flight plan checker may find the original plugin's versatility more useful.'

This plugin is still in active development, and may still contain bugs in the logic and general instability. Controllers, and especially new trainees, should not rely on this plugin for all flight plan validation.
While it will greatly aid delivery controllers, it **does not negate the need for delivery controllers to thoroughly check every flight plan before release.** It cannot provide a solution to more major route issues.

## How to use:
- Load up the plugin
- Add Tag Item type & function to Departure List

If you get an error on load, please install the [latest C++ redistributables](https://aka.ms/vs/17/release/vc_redist.x86.exe)

### How to define configurations
Examples can be found in the given Sid.json file.
