@echo off

if "%1" == "multitool" link /debug /out:multitool.exe multitool.obj libdis/*.obj

if "%1" == "debug" link /incremental:no /debug /opt:ref /opt:icf /out:multitool.exe multitool.obj libdis/*.obj

