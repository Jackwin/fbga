# fbga
A design based on Zynq-7020, utilizing AD9826 and G11620

Hierarchy:
1. project => Vivado project
2. script => A tcl script that controls the whole project
3. sdk => Include the hardware design and BSP
    1. Export the hardware design to sdk
    2. Launch SDK and choose the path to sdk which will generate the zynq_top_hw_platform_0
    3. In SDK, generate a BSP named as system_bsp, which is dependent by C project
 4. source => Include ip, rtl and block design
 5. sw => C project
