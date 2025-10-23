# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct D:\ducpm26\vivado\demo_zedboard\zedb\platform.tcl
# 
# OR launch xsct and run below command.
# source D:\ducpm26\vivado\demo_zedboard\zedb\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {zedb}\
-hw {D:\ducpm26\vivado\demo_zedboard\demo_zed.xsa}\
-proc {ps7_cortexa9_0} -os {ucos} -out {D:/ducpm26/vivado/demo_zedboard}

platform write
platform generate -domains 
platform active {zedb}
bsp reload
bsp config stdin "ps7_uart_1"
bsp config stdout "ps7_uart_1"
bsp setlib -name ucos_common -ver 1.45
bsp setlib -name ucos_osiii -ver 1.45
bsp write
bsp reload
catch {bsp regenerate}
bsp reload
bsp reload
platform clean
domain active {zynq_fsbl}
bsp reload
bsp reload
platform clean
platform config -remove-boot-bsp
platform write
platform clean
platform generate
platform clean
platform generate
platform generate -domains ucos_domain 
platform active {zedb}
bsp reload
platform generate -domains 
platform generate -domains ucos_domain 
platform active {zedb}
bsp reload
platform generate -domains 
platform active {zedb}
bsp reload
platform generate -domains 
platform generate -domains ucos_domain 
platform generate -domains ucos_domain 
platform active {zedb}
domain create -name {freertos_demo} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos_demo} -desc {} -runtime {cpp}
platform active {zedb}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
platform active {zedb}
domain create -name {freertos_zedb} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos_zedb} -desc {} -runtime {cpp}
domain create -name {freertos_zedb} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos_zedb} -desc {} -runtime {cpp}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
platform clean
bsp reload
domain create -name {freertos_zedb} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos_zedb} -desc {} -runtime {cpp}
platform active {zedb}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_1} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
platform generate
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
domain create -name {freertos} -os {freertos} -proc {ps7_cortexa9_0} -arch {32-bit} -display-name {freertos} -desc {} -runtime {cpp}
platform generate -domains 
platform write
domain -report -json
bsp reload
domain remove freertos
platform generate -domains 
platform write
platform clean
platform generate
platform active {zedb}
bsp reload
bsp reload
platform generate -domains 
platform clean
platform generate
