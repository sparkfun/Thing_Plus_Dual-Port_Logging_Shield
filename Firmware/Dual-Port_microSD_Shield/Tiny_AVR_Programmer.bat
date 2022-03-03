@echo Programming the Dual-Port microSD Shield. If this looks incorrect, abort and retry.
@pause
:loop
@echo Flashing bootloader and firmware...
@avrdude -C avrdude.conf -pattiny841 -cusbtiny -e -Uefuse:w:0xEB:m -Uhfuse:w:0xDD:m -Ulfuse:w:0x42:m -Uflash:w:Dual_mSD.hex:i
@echo Done programming! Move on to the next board.
@pause
goto loop
