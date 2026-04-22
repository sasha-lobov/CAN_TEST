import socket
import struct

sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
sock.bind(('vcan0',))

buf = bytearray()
tid_exp = None

while True:
    frame = sock.recv(72)
    if len(frame) < 8: continue

    can_id = struct.unpack_from('<I', frame, 0)[0]
    dlc = frame[4]
    data = frame[8:8+dlc]

    # Filter: Subject 3840, Node 125
    if can_id != 0x900F007D or not data:
        continue

    hdr = data[0]
    is_sof = bool(hdr & 0x80)
    is_eof = bool(hdr & 0x40)
    tid = hdr & 0x1F

    if is_sof:
        buf = bytearray(data[1:])
        tid_exp = tid
    elif tid_exp is not None and tid == tid_exp:
        buf.extend(data[1:])
        if is_eof:
            if len(buf) >= 18:
                v = int.from_bytes(buf[6:8], 'little') / 1000.0
                c = int.from_bytes(buf[8:10], 'little', signed=True) / 1000.0
                t = int.from_bytes(buf[10:12], 'little', signed=True) / 100.0
                print(f"Voltage: {v:.2f} V, Current: {c:.2f} A, Temp: {t:.2f} C")
            buf = bytearray()
            tid_exp = None
