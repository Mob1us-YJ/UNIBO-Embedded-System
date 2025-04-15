package esiot.module_lab_3_2;

import io.vertx.core.AbstractVerticle;
import io.vertx.core.Vertx;
import io.vertx.core.json.JsonArray;
import io.vertx.core.json.JsonObject;
import io.vertx.ext.web.client.WebClient;

public class TestClient extends AbstractVerticle {
    
    public static void main(String[] args) throws Exception {
        String host = "localhost"; 
        int port = 8080;

        Vertx vertx = Vertx.vertx();
        WebClient client = WebClient.create(vertx);

        // 每 2 秒发送一次数据，持续 30 秒
        int sendInterval = 2000; // 2 秒
        int totalDuration = 30000; // 30 秒

        vertx.setPeriodic(sendInterval, timerId -> {
            JsonObject item = new JsonObject();
            item.put("value", 20 + Math.random() * 5); // 生成 20~25 之间的随机温度值
            item.put("place", "lab-room"); // 设定一个明确的地点

            System.out.println("Posting new data item... " + item.encode());

            client.post(port, host, "/api/data")
                .sendJson(item)
                .onSuccess(response -> {
                    System.out.println("Posting - Received response with status code: " + response.statusCode());
                })
                .onFailure(err -> System.out.println("Posting failed: " + err.getMessage()));
        });

        // 每 5 秒获取一次数据
        vertx.setPeriodic(5000, timerId -> {
            System.out.println("Getting data items... ");
            client.get(port, host, "/api/data")
                .send()
                .onSuccess(res -> {
                    System.out.println("Getting - Received response with status code: " + res.statusCode());
                    JsonArray response = res.bodyAsJsonArray();
                    System.out.println(response.encodePrettily());
                })
                .onFailure(err -> System.out.println("Getting failed: " + err.getMessage()));
            
            // 获取系统状态
            System.out.println("Getting system status...");
            client.get(port, host, "/api/status")
                .send()
                .onSuccess(statusRes -> {
                    System.out.println("System Status - Received response with status code: " + statusRes.statusCode());
                    JsonObject status = statusRes.bodyAsJsonObject();
                    System.out.println("Status: " + status.encodePrettily());
                })
                .onFailure(err -> System.out.println("Status fetch failed: " + err.getMessage()));
        });

        // 30 秒后自动停止程序
        vertx.setTimer(totalDuration, timerId -> {
            System.out.println("Stopping test after " + (totalDuration / 1000) + " seconds.");
            vertx.close();
        });
    }
}
