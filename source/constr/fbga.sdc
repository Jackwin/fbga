
set_property PACKAGE_PIN C20 [get_ports  {led[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[0]}]
set_property PACKAGE_PIN D20 [get_ports  {led[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[1]}]
set_property PACKAGE_PIN D18 [get_ports  {led[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[2]}]
#set_property PACKAGE_PIN C19 [get_ports  {led[3]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {led[3]}]


set_property PACKAGE_PIN R7 [get_ports  {gpio_led_0_tri_o[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {gpio_led_0_tri_o[0]}]
set_property PACKAGE_PIN C19 [get_ports  {gpio_led_0_tri_o[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {gpio_led_0_tri_o[1]}]

set_property PACKAGE_PIN Y9 [get_ports  {clk50m_in}]
set_property IOSTANDARD LVCMOS33 [get_ports {clk50m_in}]
set_property PACKAGE_PIN A18 [get_ports  {rstn_in}]
set_property IOSTANDARD LVCMOS33 [get_ports {rstn_in}]

##BANK 33
#IO2_10P B13_L14_P
set_property PACKAGE_PIN AA7 [get_ports  {clk_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {clk_o}]
#IO2_8N B13_L10_N -> B35_15_p
set_property PACKAGE_PIN M21 [get_ports  {bit_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {bit_o}]
#IO2_12P B13_L16_P -> B34_15_N
set_property PACKAGE_PIN M22 [get_ports  {bit_valid_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {bit_valid_o}]

## BANK 33 AD9826
#B33_L24_P -> B33_L21_N
set_property PACKAGE_PIN Y15 [get_ports  {adc_oeb}]
set_property IOSTANDARD LVCMOS33 [get_ports {adc_oeb}]
#B33_L13_P -> B33_L24_P
set_property PACKAGE_PIN AB14 [get_ports  {adc_clk_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {adc_clk_o}]
#B33_L7_P -> B33_L7_N
set_property PACKAGE_PIN AB22 [get_ports  {adc_data_in[0]}]
#B33_L7_N -> B33_L7_P
set_property PACKAGE_PIN AA22 [get_ports  {adc_data_in[1]}]
#B33_L17_P
set_property PACKAGE_PIN AA17 [get_ports  {adc_data_in[2]}]
#B33_L17_N
set_property PACKAGE_PIN AB17 [get_ports  {adc_data_in[3]}]
#B33_L16_P -> B33_L16_N
set_property PACKAGE_PIN V17 [get_ports  {adc_data_in[4]}]
#B33_L16_N -> B33_L16_P
set_property PACKAGE_PIN U17 [get_ports  {adc_data_in[5]}]
#B33_L6_P
set_property PACKAGE_PIN V18 [get_ports  {adc_data_in[6]}]
#B33_L6_N
set_property PACKAGE_PIN V19 [get_ports  {adc_data_in[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {adc_data_in[*]}]

#B33_L14_N
set_property PACKAGE_PIN Y16 [get_ports  {sload_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {sload_o}]
#B33_L22_N
set_property PACKAGE_PIN AA14 [get_ports  {sdata_io}]
set_property IOSTANDARD LVCMOS33 [get_ports {sdata_io}]
#B33_L22_P
set_property PACKAGE_PIN Y14 [get_ports  {sclk_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {sclk_o}]

## G11620

#B33_L18_N -> B33_L13_N w18 V1.0
#B33_L12_P Y18
set_property PACKAGE_PIN Y18 [get_ports  {g11620_start_in}]
set_property IOSTANDARD LVCMOS33 [get_ports {g11620_start_in}]

#B33_L12_N -> B33_L12_P Y18 V1.0
#B33_L13_N W17 v2.0
set_property PACKAGE_PIN W18 [get_ports  {g11620_reset_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {g11620_reset_o}]
#B33_L12_P -> B33_L13_P
set_property PACKAGE_PIN W17 [get_ports  {g11620_clk_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {g11620_clk_o}]

#B33_L18_P -> B33_L18_N
set_property PACKAGE_PIN AB16 [get_ports  {g11620_cf_sel2_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {g11620_cf_sel2_o}]

#J15 pin5  V1.0
#B33_L18_P V2.0
set_property PACKAGE_PIN AA16 [get_ports  {cdsclk2_o}]
set_property IOSTANDARD LVCMOS33 [get_ports {cdsclk2_o}]


#create_generated_clock -name adcclk [get_pins clk_gen_inst/adcclk_reg/Q] -divide_by 40 -source [get_pins clk_wiz_inst/inst/clkout1_buf/O]
create_generated_clock -name adcclk -source [get_pins clk_wiz_inst/clk_200m] -divide_by 40 [get_pins clk_gen_inst/adcclk_reg/Q]







