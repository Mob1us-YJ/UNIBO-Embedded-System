import serial
import time
import tkinter as tk
#arduino = serial.Serial('COM5',9600,timeout=1)

class ArduinoClient:
    def __init__(self, port, baudRate):
        self.port = port
        self.baudRate = baudRate
        self.arduino = serial.Serial(self.port, self.baudRate, timeout=1 )

    def read_data(self):
        """读取 Arduino 发送的温度和距离数据"""
        while True:
            # 读取一行串口数据并解码
            recSer = self.arduino.readline().decode("ASCII").strip()
            if recSer:
                # 检查是否包含温度和距离的关键字
                if recSer.startswith("TEMP:") and "DIST:" in recSer:
                    # 分割数据并提取温度和距离
                    try:
                        temp_str, dist_str = recSer.split(',')
                        temperature = float(temp_str.split(':')[1])
                        distance = float(dist_str.split(':')[1])
                        return temperature, distance
                    except (ValueError, IndexError):
                        print("Error parsing data:", recSer)
            time.sleep(0.5)  # 增加延迟，以便于 Arduino 发送数据的时间

    def send_empty_command(self):
                """发送清空垃圾桶的命令"""
                self.arduino.write(b"EMPTY\n")
                print("Sent command: EMPTY")

    def send_restore_command(self):
                """发送恢复系统的命令"""
                self.arduino.write(b"RESTORE\n")
                print("Sent command: RESTORE")
    # def read_distance(self) ->float:
    #     while True:
    #         recSer = self.arduino.readline().decode("ASCII")
    #         if recSer:
    #             sonarDis = float(recSer)
    #             time.sleep(0.5)
    #             return sonarDis
if __name__ == '__main__':
    arduino = ArduinoClient('COM5',9600)
    arduino.read_data()

