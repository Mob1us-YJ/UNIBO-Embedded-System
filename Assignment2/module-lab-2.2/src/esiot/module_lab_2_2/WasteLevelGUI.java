package esiot.module_lab_2_2;

import javax.swing.*;
import java.awt.*;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

public class WasteLevelGUI extends JFrame {

    private JLabel distanceLabel;
    private JLabel temperatureLabel;
    private JLabel statusLabel;
    private JLabel fillPercentageLabel;
    private JButton emptyButton;
    private JButton restoreButton;
    private JTextArea historyArea;
    private SerialCommChannel channel;

    private List<String> history = new ArrayList<>();

    private final float MAX_TEMP = 26.0f;
    private final int MAX_TEMP_TIME = 5000; // Exceeds the maximum set duration (in milliseconds) at the specified temperature
    private final int EMPTY_THRESHOLD = 90;
    private boolean needsEmpty = false;
    private boolean needsRestore = false; // Track whether a restore operation is needed
    private long overheatStartTime = 0;

    // Time Formatter
    private final DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    public WasteLevelGUI() {
        super("Smart Waste Disposal Dashboard");

        setLayout(new BorderLayout());
        setSize(500, 500);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        // Top Information Panel
        JPanel infoPanel = new JPanel(new GridLayout(5, 1));
        distanceLabel = new JLabel("Waste Distance: N/A", SwingConstants.CENTER);
        temperatureLabel = new JLabel("Temperature: N/A", SwingConstants.CENTER);
        fillPercentageLabel = new JLabel("Fill Percentage: 0%", SwingConstants.CENTER);
        statusLabel = new JLabel("Status: OK", SwingConstants.CENTER);
        statusLabel.setForeground(Color.GREEN);

        // Empty and Restore buttons
        emptyButton = new JButton("EMPTY");
        restoreButton = new JButton("RESTORE");

        emptyButton.setEnabled(false);
        restoreButton.setEnabled(false);

        emptyButton.addActionListener(e -> sendEmptyCommand());
        restoreButton.addActionListener(e -> sendRestoreCommand());

        infoPanel.add(distanceLabel);
        infoPanel.add(temperatureLabel);
        infoPanel.add(fillPercentageLabel);
        infoPanel.add(statusLabel);
        infoPanel.add(emptyButton);
        infoPanel.add(restoreButton);

        // Middle Area for Displaying History
        historyArea = new JTextArea();
        historyArea.setEditable(false);
        JScrollPane scrollPane = new JScrollPane(historyArea);

        add(infoPanel, BorderLayout.NORTH);
        add(scrollPane, BorderLayout.CENTER);

        // Connect to Serial Port
        try {
            channel = new SerialCommChannel("COM5", 9600); // Ensure the Serial Port Number and Baud Rate are Correct
            System.out.println("Connected to Arduino.");
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Failed to connect to Arduino", "Error", JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
        }

        // Start Data Reception Thread
        new Thread(this::receiveData).start();
        setVisible(true);
    }

    private void receiveData() {
        try {
            while (true) {
                String message = channel.receiveMsg();
                System.out.println("Received message: " + message);

                SwingUtilities.invokeLater(() -> processReceivedMessage(message));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void processReceivedMessage(String message) {
        // If already in a high-temperature state without recovery and no RESTORE_ACK has been received, do not update the status further
        if (needsRestore && !message.equals("RESTORE_ACK")) {
            return;
        }

        if (message.startsWith("FILL:") && message.contains("TEMP:")) {
            try {
                String[] parts = message.split(" ");
                String fillPart = parts[0].split(":")[1].replace("%", "").trim();
                String tempPart = parts[1].split(":")[1].replace("C", "").trim();

                int fillPercentage = Integer.parseInt(fillPart);
                float temperature = Float.parseFloat(tempPart);

                fillPercentageLabel.setText("Fill Percentage: " + fillPercentage + "%");
                temperatureLabel.setText("Temperature: " + temperature + " °C");

                addHistory("Fill: " + fillPercentage + "%, Temperature: " + temperature + " °C");

                if (fillPercentage > EMPTY_THRESHOLD) {
                    needsEmpty = true;
                    statusLabel.setText("Status: Needs Emptying");
                    statusLabel.setForeground(Color.RED);
                    emptyButton.setEnabled(true);
                } else {
                    needsEmpty = false;
                    emptyButton.setEnabled(false);
                    if (!needsRestore) {
                        statusLabel.setText("Status: OK");
                        statusLabel.setForeground(Color.GREEN);
                    }
                }

                if (temperature > MAX_TEMP) {
                    if (overheatStartTime == 0) {
                        overheatStartTime = System.currentTimeMillis();
                    } else if (System.currentTimeMillis() - overheatStartTime > MAX_TEMP_TIME) {
                        needsRestore = true;
                        statusLabel.setText("Status: Overheat - Restore Needed");
                        statusLabel.setForeground(Color.RED);
                        restoreButton.setEnabled(true);
                    }
                } else {
                    overheatStartTime = 0;
                    if (!needsRestore) {
                        restoreButton.setEnabled(false);
                        if (!needsEmpty) {
                            statusLabel.setText("Status: OK");
                            statusLabel.setForeground(Color.GREEN);
                        }
                    }
                }
            } catch (Exception e) {
                System.out.println("Error processing message: " + message);
                e.printStackTrace();
            }
        } else if (message.equals("OVERHEAT_WARNING")) {
            statusLabel.setText("Status: Overheat - Intervention Required");
            statusLabel.setForeground(Color.RED);
            restoreButton.setEnabled(true);
            addHistory("System entered overheat state. Intervention required.");
            needsRestore = true; // set into "need restore"
        } else if (message.equals("CONTAINER_FULL")) {
            statusLabel.setText("Status: Container Full - Please Empty");
            statusLabel.setForeground(Color.RED);
            emptyButton.setEnabled(true);
            addHistory("Status: Container Full");
        } else if (message.equals("RESTORE_ACK")) {
            needsRestore = false;
            restoreButton.setEnabled(false);
            statusLabel.setText("Status: OK");
            statusLabel.setForeground(Color.GREEN);
            addHistory("System restored to normal state.");
        }
    }

    private void sendEmptyCommand() {
        try {
            channel.sendMsg("EMPTY");
            System.out.println("Sent EMPTY command to Arduino.");
            addHistory("Sent EMPTY command to Arduino.");
            emptyButton.setEnabled(false);
            needsEmpty = false;
            statusLabel.setText("Status: Empty Command Sent");
            statusLabel.setForeground(Color.BLUE);
        } catch (Exception e) {
            System.out.println("Failed to send EMPTY command.");
            e.printStackTrace();
        }
    }

    private void sendRestoreCommand() {
        try {
            channel.sendMsg("RESTORE");
            System.out.println("Sent RESTORE command to Arduino.");
            addHistory("Sent RESTORE command to Arduino.");
            restoreButton.setEnabled(false);
            needsRestore = false;
            statusLabel.setText("Status: Restore Command Sent");
            statusLabel.setForeground(Color.BLUE);
        } catch (Exception e) {
            System.out.println("Failed to send RESTORE command.");
            e.printStackTrace();
        }
    }

    private void addHistory(String record) {
        String timestamp = LocalDateTime.now().format(dtf);
        String recordWithTimestamp = "[" + timestamp + "] " + record;

        history.add(recordWithTimestamp);
        historyArea.append(recordWithTimestamp + "\n");
    }

    private void saveHistoryToFile() {
        try (FileWriter writer = new FileWriter("history_log.txt", true)) {
            for (String record : history) {
                writer.write(record + "\n");
            }
            writer.write("\n--- End of Session ---\n\n");
            writer.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(WasteLevelGUI::new);
    }
}
