package esiot.module_lab_3_2;

import io.vertx.core.AbstractVerticle;
import io.vertx.core.http.HttpServerResponse;
import io.vertx.core.json.JsonArray;
import io.vertx.core.json.JsonObject;
import io.vertx.ext.web.Router;
import io.vertx.ext.web.RoutingContext;
import io.vertx.ext.web.handler.BodyHandler;
import io.vertx.ext.web.handler.CorsHandler;
import io.vertx.core.http.HttpMethod;
import java.util.Set;
import java.util.HashSet;
import java.util.LinkedList;

/*
 * Data Service as a vertx event-loop 
 */
public class DataService extends AbstractVerticle {

    private int port;
    private static final int MAX_SIZE = 10; 
    private LinkedList<DataPoint> values;
    private JsonObject lastControlCommand = new JsonObject();
    
    private String systemMode = "MANUAL";
    private boolean alarmState = false; 
    private int windowOpen = 0;
    private String systemState = "NORMAL";

    public DataService(int port) {
        values = new LinkedList<>();        
        this.port = port;
    }

    @Override
    public void start() {        
        Router router = Router.router(vertx);
        
        // alow CORS 
        Set<HttpMethod> allowedMethods = new HashSet<>();
        allowedMethods.add(HttpMethod.GET);
        allowedMethods.add(HttpMethod.POST);
        allowedMethods.add(HttpMethod.OPTIONS);

        router.route().handler(CorsHandler.create("*")  
            .allowedMethods(allowedMethods)            
            .allowedHeader("Content-Type")             
            .allowedHeader("Authorization"));
        
        router.route().handler(BodyHandler.create());
        router.post("/api/data").handler(this::handleAddNewData);
        router.get("/api/data").handler(this::handleGetData);    
        router.get("/api/status").handler(this::handleGetStatus);
        router.post("/api/control").handler(this::handleControl);
        router.get("/api/control").handler(this::handleGetControl);


        vertx
            .createHttpServer()
            .requestHandler(router)
            .listen(port);    
        log("Service ready on port: " + port);
    }

    /*
     * POST/api/data， receive  data from MQTT server
     */
    private void handleAddNewData(RoutingContext routingContext) {
        HttpServerResponse response = routingContext.response();
        JsonObject res = routingContext.getBodyAsJson();
        if (res == null) {
            sendError(400, response);
        } else {
            float temperature = res.getFloat("temperature");
            int windowOpening = res.getInteger("window_opening");
            String state = res.getString("current_state");
            boolean alarm = res.getBoolean("alarm"); 

            long time = System.currentTimeMillis();
            
        
            log("Received new data: " + temperature + ", State: " + state + ", Alarm: " + alarm);
            values.addFirst(new DataPoint(temperature, time, windowOpening, state, alarm));
            if (values.size() > MAX_SIZE) {
                values.removeLast();
            }

            this.windowOpen = windowOpening;
            this.systemState = state;
            this.alarmState = alarm;  
            
            response.setStatusCode(200).end();
        }
    }

    /*
     * GET /api/data，update the data back to MQTT
     */
    private void handleGetData(RoutingContext routingContext) {
        JsonArray arr = new JsonArray();
        for (DataPoint p: values) {
            JsonObject data = new JsonObject();
            data.put("time", p.getTime());
            data.put("temperature", p.getTemperature());
            data.put("window_opening", p.getWindowOpening());
            data.put("state", p.getState());
            data.put("alarm", p.isAlarm());  // 发送警报状态
            arr.add(data);
        }
        routingContext.response()
            .putHeader("content-type", "application/json")
            .end(arr.encodePrettily());
    }

    /*
     * GET /api/status, update the system status
     */
    private void handleGetStatus(RoutingContext routingContext) {
        JsonObject status = new JsonObject();
        status.put("mode", systemMode);
        routingContext.response()
            .putHeader("content-type", "application/json")
            .end(status.encodePrettily());
    }

    /*
     * POST /api/control，set command from the front-end
     */
    private void handleControl(RoutingContext routingContext) {
        HttpServerResponse response = routingContext.response();
        JsonObject res = routingContext.getBodyAsJson();
        if (res == null) {
            sendError(400, response);
        } else {
            String command = res.getString("command");
            lastControlCommand = res.copy(); 
            JsonObject reply = new JsonObject();
            if ("TOGGLE_MODE".equals(command)) {
                systemMode = "AUTOMATIC".equals(systemMode) ? "MANUAL" : "AUTOMATIC";
                log("System mode changed to: " + systemMode);
                reply.put("status", "success").put("mode", systemMode);
            } else if ("ACK_ALARM".equals(command)) {
                alarmState = false;
                systemState = "NORMAL"; 
                log("Alarm acknowledged");
                reply.put("status", "success").put("alarm", alarmState);
                if (!values.isEmpty()) {
                    DataPoint latest = values.getFirst();
                    values.set(0, new DataPoint(
                        latest.getTemperature(), 
                        latest.getTime(), 
                        latest.getWindowOpening(), 
                        "NORMAL", // set status to normal
                        false // clear alarm
                    ));
                }
            } else if ("SET_WINDOW".equals(command)) {
                windowOpen = res.getInteger("value", 0);
                log("Window opening set to: " + windowOpen + "%");
                reply.put("status", "success").put("window", windowOpen); //set window opening in manual mode
            }
            response.putHeader("Content-Type", "application/json");
            response.setStatusCode(200).end(reply.encodePrettily());
        }
    }
    /*
     * GET /api/control，get the last control command
     */
    private void handleGetControl(RoutingContext routingContext) {
        routingContext.response()
            .putHeader("content-type", "application/json")
            .end(lastControlCommand.encodePrettily());
    }

    
    private void sendError(int statusCode, HttpServerResponse response) {
        response.setStatusCode(statusCode).end();
    }

    private void log(String msg) {
        System.out.println("[DATA SERVICE] "+msg);
    }

    /*
     * DataPoint
     */
    private static class DataPoint {
        private double temperature;
        private long time;
        private int windowOpening;
        private String state;
        private boolean alarm;  

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
}
