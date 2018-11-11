# This script sets up a Vivado project with all ip references resolved.
set proj_name "fbga"
set family "xc7z020clg484-2"

file delete -force ../project/${proj_name}.xpr ../project/*.os ../project/*.jou ../project/bd
file delete -force ../project/*.log ../project/${proj_name}.srcs ../project/${proj_name}.cache ${proj_name}.runs


create_project -force ../project/${proj_name} -part ${family}

set top_module_name "zynq_top"

set_property target_language verilog [current_project]
set_property default_lib work [current_project]
load_features ipintegrator
tclapp::install ultrafast -quiet


puts "Set up Zynq system"

cd ../source
source ./zynq_sys_bd.tcl
generate_target {synthesis implementation} [get_files ./bd/zynq_sys/zynq_sys.bd]
make_wrapper -files [get_files ./bd/zynq_sys/zynq_sys.bd] -top
read_verilog ./bd/zynq_sys/hdl/zynq_sys_wrapper.v

cd ../script

 ## --------------- Add files to Vivado project ----------------------

puts "Start to add files to Vivado project"

## ---------------- In batch mode ---------------------------------------------
puts "Start to compile in batch mode"
# Source code
read_verilog [glob ../source/rtl/adc9826.v]
read_verilog [glob ../source/rtl/adc9826_cfg.v]
read_verilog [glob ../source/rtl/clk_gen.v]
read_verilog [glob ../source/rtl/data_gen.v]
read_verilog [glob ../source/rtl/g11620.v]
read_verilog [glob ../source/rtl/oserdes.v]
read_verilog [glob ../source/rtl/param.h]
read_verilog [glob ../source/rtl/reset_bridge.v]
read_verilog [glob ../source/rtl/serial_interface.v]
read_verilog [glob ../source/rtl/serializer.v]
read_verilog [glob ../source/rtl/zynq_top.v]
read_verilog [glob ../source/bd/zynq_sys/hdl/zynq_sys_wrapper.v]
read_xdc  ../source/constr/fbga.sdc

read_ip ../source/ip/clk_wiz/clk_wiz.xci
read_ip ../source/ip/dpbram_8inx256/dpbram_8inx256.xci
read_ip ../source/ip/ila_ad9826/ila_ad9826.xci
read_ip ../source/ip/ila_adc/ila_adc.xci
read_ip ../source/ip/ila_clk_gen/ila_clk_gen.xci
read_ip ../source/ip/ila_g11620/ila_g11620.xci
read_ip ../source/ip/vio_cfg_9826/vio_cfg_9826.xci
read_ip ../source/ip/vio_data_gen/vio_data_gen.xci
read_ip ../source/ip/vio_g11620/vio_g11620.xci
read_ip ../source/ip/vio_reg/vio_reg.xci
read_ip ../source/ip/ila_data_gen/ila_data_gen.xci

generate_target {all} [get_ips *]

update_ip
read_bd ../source/bd/zynq_sys/zynq_sys.bd


close_project




