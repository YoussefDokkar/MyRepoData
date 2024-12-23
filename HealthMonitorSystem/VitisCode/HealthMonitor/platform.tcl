# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\HealthMonitorSystem\VitisCode\HealthMonitor\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\HealthMonitorSystem\VitisCode\HealthMonitor\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {HealthMonitor}\
-hw {C:\HealthMonitorSystem\Hardware_final_wrapper.xsa}\
-proc {microblaze_0} -os {standalone} -out {C:/HealthMonitorSystem/VitisCode}

platform write
platform generate -domains 
platform active {HealthMonitor}
platform generate
platform clean
platform generate
platform clean
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {HealthMonitor}
platform config -updatehw {C:/HealthMonitorSystem/Hardware_final_extraMemory.xsa}
platform generate -domains 
platform clean
platform generate
