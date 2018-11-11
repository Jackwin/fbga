## FBGA
A design based on Zynq-7020, utilizing AD9826 and G11620

## Hierarchy
1. project => Vivado project
2. script => A tcl script that controls the whole project
3. sdk => Include the hardware design and BSP
    1. Export the hardware design to sdk
    2. Launch SDK and choose the path to sdk which will generate the zynq_top_hw_platform_0
    3. In SDK, generate a BSP named as system_bsp, which is dependent by C project
 4. source => Include ip, rtl and block design
 5. sw => C project

## Usage
 1. git clone https://github.com/Jackwin/fbga.git
 2. Use Vivado 2017.4 to open the peoject, and the hierarchy will be built automatically.
 3. From the File panel of Vivado, choose Launch SDK, where the Exported location and Workspace both choose the sdk folder.
 4. When SDK is launched, a HW design named as "zynq_top_hw_platform_0" will be shown, based on which, make a "system_bsp" BSP that is required 
	by the application project.
 5. In the SDK, import the C project from the sw folder via File->Import->General->Existing Projects.