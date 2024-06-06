import serial


with serial.Serial("/dev/ttyUSB0", 9600) as ser:
    while 1:
        n = ser.read()[0] + (ser.read()[0] << 8)
        print("Recebido {:6d}".format(n))
