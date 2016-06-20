# 370GT
##### Vehicle Remote Compute Unit (Prototype)

### Environment
Currently using [PlatformIO](http://platformio.org/) for building. A lot of the files are `.cpp` but only to use the internal Arduino libs. 99% of the project is written in C. The project will probably be migrated to Atmel Studio after prototype is complete.

### Hardware
Currently using these shields:
- Embedded Artists 3G+GPS Shield (SARA-U270) on __TX0/RX0__ [[product page]](http://www.embeddedartists.com/products/acc/cell_3g_pos_shield.php)
- Freematics OBD-II UART (ELM327/STN1110) on __TX2/RX2__  [[product page]](http://freematics.com/pages/products/freematics-obd-ii-uart-adapter-mk2/)
