package esiot.module_lab_3_2;

import io.vertx.core.Vertx;
import io.vertx.core.VertxOptions;
import java.util.concurrent.TimeUnit;

public class TestMQTTClient {
    public static void main(String[] args) throws Exception {		
        // creat Vert.x, set max worker execute time to 10 minutes
        VertxOptions options = new VertxOptions()
                .setMaxWorkerExecuteTime(TimeUnit.MINUTES.toNanos(10));
        Vertx vertx = Vertx.vertx(options);
        
        // deploy MQTTAgent verticle
        MQTTAgent agent = new MQTTAgent();
        vertx.deployVerticle(agent);
        
        System.out.println("Deployed MQTTAgent on Vert.x instance: " + vertx.hashCode());
    }
}
