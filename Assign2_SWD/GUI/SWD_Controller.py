import threading
import time
import tkinter
import queue

from SWD_GUI import Win
from Arduino_Client import ArduinoClient
from tkinter import messagebox



class Controller:
    # 导入UI类后，替换以下的 object 类型，将获得 IDE 属性提示功能
    ui: Win

    def __init__(self, ard_client):
        self.data_thread = None
        self.arduino_client = ard_client
        self.data_queue = queue.Queue()

    def init(self, ui):
        self.ui = ui
        self.update_data()
        # 启动线程持续读取 Arduino 数据
        self.data_thread = threading.Thread(target=self.read_data_thread, daemon=True)
        self.data_thread.start()

    def Empty_bin(self,evt):
        self.arduino_client.send_empty_command()
        messagebox.showinfo("Info", "Empty command sent to Arduino.")

    def Restore_bin(self,evt):
        self.arduino_client.send_restore_command()
        messagebox.showinfo("Info", "Restore command sent to Arduino.")

    def update_data(self):
        """定期从队列中获取数据并更新 UI"""
        try:
            # 从队列中获取数据（非阻塞）
            temp, dis = self.data_queue.get_nowait()
            if temp is not None:
                self.ui.tk_text_Temp_text.delete('1.0', 'end')
                self.ui.tk_text_Temp_text.insert('end', f"{temp:.2f} °C")
            if dis is not None:
                self.ui.tk_text_Filling_Percent_text.delete('1.0', 'end')
                self.ui.tk_text_Filling_Percent_text.insert('end', f"{dis:.2f} cm")
        except queue.Empty:
            pass

        # 每秒更新一次数据
        self.ui.after(1000, self.update_data)

    def read_data_thread(self):
            """后台线程从 Arduino 读取数据并放入队列"""
            while True:
                temp, dis = self.arduino_client.read_data()
                if temp is not None and dis is not None:
                    # 将数据放入队列，供主线程更新 UI
                    self.data_queue.put((temp, dis))
                time.sleep(1)  # 降低读取频率以防止过度占用资源
