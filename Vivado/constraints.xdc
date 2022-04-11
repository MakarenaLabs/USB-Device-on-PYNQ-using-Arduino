set_property -dict {PACKAGE_PIN Y11 IOSTANDARD LVCMOS33} [get_ports gpio_rtl_tri_o]
set_property -dict {PACKAGE_PIN Y12 IOSTANDARD LVCMOS33} [get_ports RESET_DEBUG]

set_property -dict {PACKAGE_PIN W14 IOSTANDARD LVCMOS33} [get_ports iic_rtl_sda_io]
set_property -dict {PACKAGE_PIN Y14 IOSTANDARD LVCMOS33} [get_ports iic_rtl_scl_io]

set_property PULLUP true [get_ports iic_rtl_scl_io]
set_property PULLUP true [get_ports iic_rtl_sda_io]