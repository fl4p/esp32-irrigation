import glob
import sys
from time import sleep

import serial # pip install pyserial

from util import get_logger

logger = get_logger()

serial_port = sorted(glob.glob('/dev/tty.usbserial-*'))[-1]

logger.info('using port %s', serial_port)
ser = serial.Serial(serial_port, baudrate=9600, timeout=2)

def query(cmd:str):
    global ser

    if not ser.is_open:
        logger.info("open serial port")
        ser.open()

    try:
        ser.reset_input_buffer()
        ser.write(bytes(cmd + '\r\n', 'ascii'))
        line = ser.readline()
    except OSError:
        logger.error(sys.exc_info(), exc_info=True)
        if ser.is_open:
            ser.close()
        return None
    if not line:
        logger.warning("no response for %s", cmd)
        return None
    return line.decode('ascii').strip()


while True:

    temp = float(query("GetTemperature!") or 'nan')
    water = float(query("GetWaterContent!") or 'nan')
    if temp>-50 or water > -1:
        logger.info("temp: %5.1f ºC   water: %5.2f %%", temp, water)

    sleep(1 if water > 0 else 10)
