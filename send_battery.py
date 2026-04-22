import dronecan
import time

node = dronecan.make_node('vcan0', node_id=3)

print("Ноду запустиииил")

while True:
    msg = dronecan.uavcan.equipment.power.BatteryInfo()

    msg.voltage = 12.1
    msg.current = 1.5
    msg.remaining_capacity_wh = 45.0

    node.broadcast(msg)
    print("отправляю BatteryInfo, чувааак")

    time.sleep(1)