from bluepy import btle
import time 


class ScanDelegate(btle.DefaultDelegate):
    def __init__(self):
        btle.DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print "Discovered device", dev.addr
        elif isNewData:
            print "Received new data from", dev.addr


class MyDelegate(btle.DefaultDelegate):
    def __init__(self, params):
        btle.DefaultDelegate.__init__(self)
        # ... initialise here

    def handleNotification(self, cHandle, data):
        print "Notificated..."
        # ... perhaps check cHandle
        # ... process 'data'


scanner = btle.Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(20.0)

id = "d2:2b:bc:9f:80:02"

for dev in devices:
    print "Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi)
    for (adtype, desc, value) in dev.getScanData():
        print "  %s = %s" % (desc, value)
    if (dev.addr == id):
      print "Get pheripheral..."
      p = btle.Peripheral(dev)
      print "got chucky"
      characteristics = p.getCharacteristics()
      print characteristics
      for characteristic in characteristics:
        print "uuid = %s , handle = %s" % (characteristic.uuid, characteristic.getHandle())
      print "setting deleg.."
      params = "foo"
      p.setDelegate( MyDelegate(params) )
      cha = p.getCharacteristics(uuid="fdd8d970-9d96-4ac7-a2e1-84d204f6523b")[0]
      print cha

      ccc_desc = cha.getDescriptors(forUUID=0x2902)[0]
      ccc_desc.write(b"\x01")
      while True:
        print "gram"
        r = cha.read()
        print "r = %s" % (r)
        if p.waitForNotifications(2):
          print "bonni"

