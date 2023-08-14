# HKCP Plugin
The HKCP (Hong Kong Controller Pluging) contains improvements and added functionality to Euroscope for Hong Kong vACC Controllers. It currently has two main functions - a flight plan checker and a missed approach alarm.

## VFPC
VFPC (Virtual Flight Plan Checker) checks flight plans against relevant route and altitude restrictions. 
Originally developed by hpeter2 and DrFreas, it has been expanded on with modified functionality to better suit the needs of HKvACC. You can find the codebase of the original plugin [here](https://github.com/hpeter2/VFPC). While it greatly aids delivery controllers, it **does not negate the need for delivery controllers to thoroughly check every flight plan before release.** It cannot provide a solution to more major route issues.

## Missed Approach Alarm
The missed approach alarm allows tower controllers to send a visual and audio alert to TMA controllers for missed approaches. When the alert is sent (via the tag or window), the relevant TMA controllers receive a popup containing the details about the missed approach aircraft (callsign, airport, runway). The TMA controller can then acknowledge the request, saving the need for extra coordination over voice or chat.

## How to use:
- Load up the plugin
- Add VFPC Tag Item type & function to Departure List

If you get an error on load, please install the [latest C++ redistributables](https://aka.ms/vs/17/release/vc_redist.x86.exe)

### How to define configurations
Examples can be found in the given Sid.json file.
