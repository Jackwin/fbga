# Script to compile the FPGA with zynq processor system all the way to bit file.
set outputDir ../project/results
file mkdir $outputDir
open_project ../project/fbga.xpr

synth_ip -quiet [get_ips *]

synth_design -top zynq_top
write_checkpoint -force $outputDir/post_synth.dcp

opt_design

place_design

phys_opt_design

route_design
write_checkpoint -force $outputDir/post_route.dcp
report_route_status -file $outputDir/post_route_status.rpt
report_timing_summary -file $outputDir/post_route_timing_summary.rpt
report_power -file $outputDir/post_route_power.rpt
report_drc -file $outputDir/post_imp_drc.rpt
report_io -file $outputDir/post_imp_io.rpt
xilinx::ultrafast::report_io_reg -verbose -file $outputDir/io_regs.rpt

write_hwdef -force  -file ../sdk/zynq_top.hdf

write_bitstream -verbose -force $outputDir/top.bit

close_project

#write_cfgmem -disablebitswap -force -format BIN -size 256 -interface SMAPx32 -loadbit "up 0x0 ./results/top.bit" -verbose ./results/top.bit.bin






