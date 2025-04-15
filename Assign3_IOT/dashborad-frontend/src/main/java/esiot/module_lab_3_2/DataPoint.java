package esiot.module_lab_3_2;

public class DataPoint {
    private double temperature;  // 传感器温度
    private long time;           // 记录时间戳
    private int windowOpening;   // 窗口开启度 (0-100%)
    private String state;        // 当前系统状态 ("Auto_NORMAL", "Auto_HOT", "Auto_TOO_HOT", "Auto_ALARM")
    private boolean alarm;       // 是否触发警报

    public DataPoint(double temperature, long time, int windowOpening, String state, boolean alarm) {
        this.temperature = temperature;
        this.time = time;
        this.windowOpening = windowOpening;
        this.state = state;
        this.alarm = alarm;
    }

    // **✅ Getters**
    public double getTemperature() {
        return temperature;
    }

    public long getTime() {
        return time;
    }

    public int getWindowOpening() {
        return windowOpening;
    }

    public String getState() {
        return state;
    }

    public boolean isAlarm() {
        return alarm;
    }

    // **✅ toString() 方法，方便日志输出**
    @Override
    public String toString() {
        return "DataPoint{" +
                "temperature=" + temperature +
                ", time=" + time +
                ", windowOpening=" + windowOpening +
                ", state='" + state + '\'' +
                ", alarm=" + alarm +
                '}';
    }
}
