package esiot.module_lab_2_2;

public class TestSensorRead {

    public static void main(String[] args) throws Exception {
        // Create a SerialCommChannel instance to connect to the specified COM port
        SerialCommChannel channel = new SerialCommChannel("COM5", 9600); // Adjust COM port as needed

        /* Waiting for Arduino to reboot */
        System.out.println("Waiting for Arduino to reboot...");
        Thread.sleep(4000);
        System.out.println("Arduino connected. Starting communication.");

        while (true) {
            // Send command to Arduino to read sensor data
            System.out.println("Sending: READ_SENSOR");
            channel.sendMsg("READ_SENSOR");

            // Receive the response data from Arduino
            String msg = channel.receiveMsg();
            System.out.println("Received sensor data: " + msg);

            // Wait for a while before reading again
            Thread.sleep(1000);
        }
    }
}

