package esiot.module_lab_3_2;

import com.fazecast.jSerialComm.SerialPort;
import io.netty.handler.codec.mqtt.MqttQoS;
import io.vertx.core.AbstractVerticle;
import io.vertx.core.Vertx;
import io.vertx.core.buffer.Buffer;
import io.vertx.core.json.JsonObject;
import io.vertx.ext.web.client.WebClient;
import io.vertx.mqtt.MqttClient;

import java.io.OutputStream;

/**
 * MQTTAgent receive temperature data from ESP32, get status from /api/status
 * Proceed data according to current state：
 * 1. Send Temp data, Current State, Window Opening to HTTP server (http://localhost:8080/api/data)
 * 2. Send data to Serial Port：
 *    - if AUTOMATIC, calculate window opening and frequency;
 *    - if MANUAL，temp-ESP32, window opening-http://localhost:8080/api/control, state-"Manual",
 *          check if ACK_ALARM received, set alarm to false.
 */
public class MQTTAgent extends AbstractVerticle {

    // MQTT Broker
    private static final String BROKER_ADDRESS = "broker.mqtt-dashboard.com";
    private static final String TEMPERATURE_TOPIC = "temperature/data";
    private static final String FREQUENCY_TOPIC = "temperature/frequency";

    // HTTP API URL
    private static final String HTTP_DATA_URL = "http://localhost:8080/api/data";
    private static final String HTTP_STATUS_URL = "http://localhost:8080/api/status";
    private static final String HTTP_CONTROL_URL = "http://localhost:8080/api/control";

    // Temperature thresholds and frequency values(Auto mode)
    private static final double T1 = 10.0;  //Hot threshold
    private static final double T2 = 15.0;  //Too hot threshold
    private static final int F1 = 5000;
    private static final int F2 = 1000;
    private static final long DT = 20000; // overheat time threshold

    // 
    private long tooHotStartTime = -1;
    private String mode = "AUTOMATIC";   
    private double lastTemperature = 0.0;  

    private MqttClient client;
    private WebClient webClient;
    private SerialPort serialPort;
    private OutputStream serialOutputStream;

    /**
     * SystemData class to store system data
     */
    private static class SystemData {
        double temperature;
        double windowOpening; 
        String state;         
        boolean alarm;
        int frequency;        
    }

    @Override
    public void start() {
        client = MqttClient.create(vertx);
        webClient = WebClient.create(vertx);

        client.connect(1883, BROKER_ADDRESS, ar -> {
            if (ar.succeeded()) {
                log("Connected to MQTT Broker");
                // dealing with temperature data from ESP32
                client.publishHandler(s -> {
                    String payload = s.payload().toString();
                    try {
                        JsonObject json = new JsonObject(payload);
                        double temperature = json.getDouble("temperature");
                        lastTemperature = temperature;
                        log("Received Temperature: " + temperature);
                        fetchSystemMode();
                    } catch (Exception e) {
                        log("Error parsing MQTT message: " + e.getMessage());
                    }
                }).subscribe(TEMPERATURE_TOPIC, 2);
            } else {
                log("Failed to connect to MQTT Broker: " + ar.cause().getMessage());
            }
        });

        connectSerialPort("COM6");
    }

    /**
     * get system mode from /api/status （"AUTOMATIC" or "MANUAL"）
     */
    private void fetchSystemMode() {
        webClient.getAbs(HTTP_STATUS_URL).send(ar -> {
            if (ar.succeeded()) {
                JsonObject response = ar.result().bodyAsJsonObject();
                mode = response.getString("mode", "AUTOMATIC");
                log("System Mode: " + mode);

                // calculate system data according to temperature
                SystemData data = computeAutoValues(lastTemperature);
                if ("AUTOMATIC".equalsIgnoreCase(mode)) {
                    // AUTOMATIC mode, check /api/control for SET_WINDOW command (and ACK_ALARM)
                    checkControlForAckAlarm(data, () -> {
                        sendFrequencyUpdate(data.frequency);
                        processData(data);
                    });
                } else { // MANUAL 
                    // MANUAL mode, check /api/control for SET_WINDOW command (and ACK_ALARM)
                    fetchControlDataForManual(data, () -> {
                        data.state = "Manual";
                        processData(data);
                    });
                }
            } else {
                log("Failed to fetch system mode: " + ar.cause().getMessage());
            }
        });
    }

    /**
     * calculate system data according to temperature(Auto mode)
     */
    private SystemData computeAutoValues(double T) {
        SystemData data = new SystemData();
        data.temperature = T;
        if (T < T1) {
            data.state = "Auto_NORMAL";
            data.windowOpening = 0.0;
            data.frequency = F1;
            data.alarm = false;
            tooHotStartTime = -1;
        } else if (T >= T1 && T <= T2) {
            data.state = "Auto_HOT";
            data.windowOpening = (T - T1) / (T2 - T1);
            data.frequency = F2;
            data.alarm = false;
            tooHotStartTime = -1;
        } else {
            data.state = "Auto_TOO_HOT";
            data.windowOpening = 1.0;
            data.frequency = F2;
            if (tooHotStartTime == -1) {
                tooHotStartTime = System.currentTimeMillis();
            }
            if (System.currentTimeMillis() - tooHotStartTime >= DT) {
                data.state = "Auto_ALARM";
                data.alarm = true;
            } else {
                data.alarm = false;
            }
        }
        return data;
    }

    /**
     * AUTOMATIC mode: check /api/control for ACK_ALARM command
     */
    private void checkControlForAckAlarm(SystemData data, Runnable completion) {
        webClient.getAbs(HTTP_CONTROL_URL).send(ar -> {
            if (ar.succeeded()) {
                try {
                    String body = ar.result().bodyAsString();
                    // 返回的数据格式为单个 JSON 对象
                    JsonObject cmd = new JsonObject(body);
                    String command = cmd.getString("command");
                    if ("ACK_ALARM".equalsIgnoreCase(command)) {
                        data.alarm = false;
                        log("ACK_ALARM received: alarm set to false");
                    }
                } catch (Exception e) {
                    log("Error parsing control data: " + e.getMessage());
                }
            } else {
                log("Failed to fetch control data: " + ar.cause().getMessage());
            }
            completion.run();
        });
    }

    /**
     * MANUAL mode: check /api/control for SET_WINDOW command or ACK_ALARM command
     */
    private void fetchControlDataForManual(SystemData data, Runnable completion) {
        webClient.getAbs(HTTP_CONTROL_URL).send(ar -> {
            if (ar.succeeded()) {
                try {
                    String body = ar.result().bodyAsString();
                    JsonObject cmd = new JsonObject(body);
                    String command = cmd.getString("command");
                    if ("SET_WINDOW".equalsIgnoreCase(command)) {
                        int value = cmd.getInteger("value", 0);
                        data.windowOpening = value / 100.0;
                        log("SET_WINDOW received: windowOpening set to " + data.windowOpening);
                    } else if ("ACK_ALARM".equalsIgnoreCase(command)) {
                        data.alarm = false;
                        log("ACK_ALARM received in manual mode: alarm set to false");
                    }
                } catch (Exception e) {
                    log("Error parsing manual control data: " + e.getMessage());
                }
            } else {
                log("Failed to fetch manual control data: " + ar.cause().getMessage());
            }
            completion.run();
        });
    }

    /**
     * Send data to Serial Port and HTTP Server
     */
    private void processData(SystemData data) {
        sendSerialData(data.temperature, data.windowOpening, data.state, data.alarm);
        sendToHttpServer(data.temperature, data.windowOpening, data.state, data.alarm);
    }

    /**
     * Send frequency update to /temperature/frequency
     */
    private void sendFrequencyUpdate(int frequency) {
        String frequencyMsg = "{\"frequency\": " + frequency + "}";
        client.publish(FREQUENCY_TOPIC, Buffer.buffer(frequencyMsg), MqttQoS.AT_LEAST_ONCE, false, false);
        log("Sent Frequency Update: " + frequency + " ms");
    }

    /**
     * Send data to Serial Port
     */
    private void sendSerialData(double temperature, double opening, String state, boolean alarm) {
        if (serialOutputStream == null) {
            log("Error: serialOutputStream is NULL, cannot send data!");
            return;
        }

        vertx.executeBlocking(promise -> {
            try {
                // build JSON object
                JsonObject dataObj = new JsonObject();
                dataObj.put("temperature", temperature);
                dataObj.put("window_opening", (int) (opening * 100));
                dataObj.put("current_state", state);
                dataObj.put("alarm", alarm);

                String jsonMsg = dataObj.encode() + "\n";
                log("Preparing to send Serial Data: " + jsonMsg);

                // Write
                synchronized (serialOutputStream) {
                    serialOutputStream.write(jsonMsg.getBytes());
                    serialOutputStream.flush();
                }

                log("Sent Serial Data successfully: " + jsonMsg);
                promise.complete();
            } catch (Exception e) {
                log("Failed to send serial data: " + e.getMessage());
                promise.fail(e);
            }
        }, res -> {
            if (res.failed()) {
                log("Serial write operation failed: " + res.cause().getMessage());
            }
        });
    }

    /**
     * Send data to HTTP Server through api/data
     */
    private void sendToHttpServer(double temperature, double opening, String state, boolean alarm) {
        JsonObject jsonData = new JsonObject()
                .put("temperature", temperature)
                .put("window_opening", (int) (opening * 100))
                .put("current_state", state)
                .put("alarm", alarm);

        webClient.postAbs(HTTP_DATA_URL)
                .putHeader("Content-Type", "application/json")
                .sendJsonObject(jsonData, ar -> {
                    if (ar.succeeded()) {
                        log("Data sent to HTTP server successfully: " + jsonData.encode());
                    } else {
                        log("Failed to send data to HTTP server: " + ar.cause().getMessage());
                    }
                });
    }

    /**
     * name portName
     */
    private void connectSerialPort(String portName) {
        serialPort = SerialPort.getCommPort(portName);
        serialPort.setBaudRate(115200);
        serialPort.setNumDataBits(8);
        serialPort.setParity(SerialPort.NO_PARITY);
        serialPort.setNumStopBits(SerialPort.ONE_STOP_BIT);

        if (serialPort.openPort()) {
            log("Serial Port " + portName + " opened successfully.");
            serialOutputStream = serialPort.getOutputStream();
        } else {
            log("Failed to open serial port.");
        }
    }

    private void log(String msg) {
        System.out.println("[MQTT AGENT] " + msg);
    }
}
