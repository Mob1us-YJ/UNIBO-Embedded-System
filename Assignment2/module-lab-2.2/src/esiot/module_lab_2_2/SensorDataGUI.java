package esiot.module_lab_2_2;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class SensorDataGUI extends JFrame {
    
    private JLabel sensorDataLabel; // Display sensor data
    private JButton startButton;
    private JButton openDoorButton, closeDoorButton; // Buttons to control door
    private SerialCommChannel channel;
    private boolean running = false;

    public SensorDataGUI() {
        // Set up frame
        super("Arduino Sensor Data and Control");
        setLayout(new BorderLayout());
        setSize(400, 250);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        // Initialize label to show sensor data
        sensorDataLabel = new JLabel("Sensor Data: N/A", SwingConstants.CENTER);
        sensorDataLabel.setFont(new Font("Arial", Font.BOLD, 24));
        add(sensorDataLabel, BorderLayout.CENTER);

        // Initialize buttons
        startButton = new JButton("Start Reading Sensor Data");
        openDoorButton = new JButton("Open Door");
        closeDoorButton = new JButton("Close Door");

        // Add button panel
        JPanel buttonPanel = new JPanel();
        buttonPanel.add(startButton);
        buttonPanel.add(openDoorButton);
        buttonPanel.add(closeDoorButton);
        add(buttonPanel, BorderLayout.SOUTH);

        // Action listener for start/stop reading button
        startButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (!running) {
                    startReadingData();
                } else {
                    stopReadingData();
                }
            }
        });

        // Action listener for Open Door button
        openDoorButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                sendCommandToArduino("OPEN");
            }
        });

        // Action listener for Close Door button
        closeDoorButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                sendCommandToArduino("CLOSE");
            }
        });
        
        // Initialize serial connection
        try {
            channel = new SerialCommChannel("COM5", 9600); // Adjust COM port as needed
            System.out.println("Connected to Arduino.");
        } catch (Exception ex) {
            JOptionPane.showMessageDialog(this, "Failed to connect to Arduino", "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    // Method to start reading data
    private void startReadingData() {
        running = true;
        startButton.setText("Stop Reading Sensor Data");

        // Start a new thread to read from Arduino
        new Thread(() -> {
            try {
                while (running) {
                    channel.sendMsg("READ_SENSOR");
                    String sensorData = channel.receiveMsg();
                    SwingUtilities.invokeLater(() -> sensorDataLabel.setText("Sensor Data: " + sensorData));
                    Thread.sleep(1000);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();
    }

    // Method to stop reading data
    private void stopReadingData() {
        running = false;
        startButton.setText("Start Reading Sensor Data");
        sensorDataLabel.setText("Sensor Data: N/A");
    }

    // Method to send command to Arduino
    private void sendCommandToArduino(String command) {
        try {
            channel.sendMsg(command);
            System.out.println("Sent command: " + command);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            SensorDataGUI gui = new SensorDataGUI();
            gui.setVisible(true);
        });
    }
}
