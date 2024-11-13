import random
from tkinter import *
from tkinter.ttk import *
class WinGUI(Tk):
    def __init__(self):
        super().__init__()
        self.__win()
        self.tk_button_Empty_Button = self.__tk_button_Empty_Button(self)
        self.tk_label_Button_Label = self.__tk_label_Button_Label(self)
        self.tk_button_Restore_Button = self.__tk_button_Restore_Button(self)
        self.tk_label_Monitor_Label = self.__tk_label_Monitor_Label(self)
        self.tk_label_m35r08ss = self.__tk_label_m35r08ss(self)
        self.tk_progressbar_Filling_Percent_bar = self.__tk_progressbar_Filling_Percent_bar(self)
        self.tk_text_Filling_Percent_text = self.__tk_text_Filling_Percent_text(self)
        self.tk_label_Temp_Label = self.__tk_label_Temp_Label(self)
        self.tk_text_Temp_text = self.__tk_text_Temp_text(self)
    def __win(self):
        self.title("OPERATOR DASHBOARD")
        # 设置窗口大小、居中
        width = 510
        height = 310
        screenwidth = self.winfo_screenwidth()
        screenheight = self.winfo_screenheight()
        geometry = '%dx%d+%d+%d' % (width, height, (screenwidth - width) / 2, (screenheight - height) / 2)
        self.geometry(geometry)

        self.resizable(width=False, height=False)

    def scrollbar_autohide(self,vbar, hbar, widget):
        """自动隐藏滚动条"""
        def show():
            if vbar: vbar.lift(widget)
            if hbar: hbar.lift(widget)
        def hide():
            if vbar: vbar.lower(widget)
            if hbar: hbar.lower(widget)
        hide()
        widget.bind("<Enter>", lambda e: show())
        if vbar: vbar.bind("<Enter>", lambda e: show())
        if vbar: vbar.bind("<Leave>", lambda e: hide())
        if hbar: hbar.bind("<Enter>", lambda e: show())
        if hbar: hbar.bind("<Leave>", lambda e: hide())
        widget.bind("<Leave>", lambda e: hide())

    def v_scrollbar(self,vbar, widget, x, y, w, h, pw, ph):
        widget.configure(yscrollcommand=vbar.set)
        vbar.config(command=widget.yview)
        vbar.place(relx=(w + x) / pw, rely=y / ph, relheight=h / ph, anchor='ne')
    def h_scrollbar(self,hbar, widget, x, y, w, h, pw, ph):
        widget.configure(xscrollcommand=hbar.set)
        hbar.config(command=widget.xview)
        hbar.place(relx=x / pw, rely=(y + h) / ph, relwidth=w / pw, anchor='sw')
    def create_bar(self,master, widget,is_vbar,is_hbar, x, y, w, h, pw, ph):
        vbar, hbar = None, None
        if is_vbar:
            vbar = Scrollbar(master)
            self.v_scrollbar(vbar, widget, x, y, w, h, pw, ph)
        if is_hbar:
            hbar = Scrollbar(master, orient="horizontal")
            self.h_scrollbar(hbar, widget, x, y, w, h, pw, ph)
        self.scrollbar_autohide(vbar, hbar, widget)
    def __tk_button_Empty_Button(self,parent):
        btn = Button(parent, text="EMPTY", takefocus=False,)
        btn.place(x=300, y=80, width=150, height=40)
        return btn
    def __tk_label_Button_Label(self,parent):
        label = Label(parent,text="Control Buttons",anchor="center", )
        label.place(x=300, y=30, width=150, height=30)
        return label
    def __tk_button_Restore_Button(self,parent):
        btn = Button(parent, text="RESTORE", takefocus=False,)
        btn.place(x=300, y=130, width=150, height=40)
        return btn
    def __tk_label_Monitor_Label(self,parent):
        label = Label(parent,text="Monitors",anchor="center", )
        label.place(x=50, y=30, width=150, height=30)
        return label
    def __tk_label_m35r08ss(self,parent):
        label = Label(parent,text="Filling Percentage",anchor="center", )
        label.place(x=50, y=80, width=110, height=30)
        return label
    def __tk_progressbar_Filling_Percent_bar(self,parent):
        progressbar = Progressbar(parent, orient=HORIZONTAL,)
        progressbar.place(x=50, y=130, width=150, height=30)
        return progressbar
    def __tk_text_Filling_Percent_text(self,parent):
        text = Text(parent)
        text.place(x=210, y=130, width=60, height=30)
        return text
    def __tk_label_Temp_Label(self,parent):
        label = Label(parent,text="Temperature",anchor="center", )
        label.place(x=50, y=180, width=110, height=30)
        return label
    def __tk_text_Temp_text(self,parent):
        text = Text(parent)
        text.place(x=50, y=230, width=140, height=30)
        return text


class Win(WinGUI):
    def __init__(self, controller):
        self.ctl = controller
        super().__init__()
        self.__event_bind()
        self.__style_config()
        self.ctl.init(self)

    def __event_bind(self):
        self.tk_button_Empty_Button.bind('<Button-1>',self.ctl.Empty_bin)
        self.tk_button_Restore_Button.bind('<Button-1>',self.ctl.Restore_bin)
        pass
    def __style_config(self):
        pass
if __name__ == "__main__":
    win = WinGUI()
    win.mainloop()