
from SWD_GUI import Win as MainWin
# from PLC_UI_control import Controller as MainUIController
from SWD_Controller import Controller as MainUIController
from Arduino_Client import ArduinoClient

SWD_client = ArduinoClient('COM5',9600)
controller = MainUIController(SWD_client)
app = MainWin(controller)

if __name__ == "__main__":
    app.mainloop()