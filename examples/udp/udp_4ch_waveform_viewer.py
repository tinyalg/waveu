import socket
import numpy as np
import pyqtgraph as pg
from PyQt6 import QtWidgets, QtCore
from PyQt6.QtCore import Qt  # For convenience
import threading
import time

# UDP Configuration
UDP_IP = "0.0.0.0"  # Listen on all interfaces
UDP_PORT = 8000    # Must match ESP32's port
BUFFER_SIZE = 6144 + 12  # Total bytes (8-byte timestamp + 16000 waveform data)
LOG_FILE = "timestamps.log"  # File to store timestamps

class UDPPlotter(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("UDP 4-Channel Waveform Viewer (Alternate Mode)")
        self.setGeometry(100, 100, 800, 600)
        
        # Create UI elements
        self.start_button = QtWidgets.QPushButton("Start", self)
        self.stop_button = QtWidgets.QPushButton("Stop", self)
        self.start_button.clicked.connect(self.start_udp)
        self.stop_button.clicked.connect(self.stop_udp)
        
        # Create 4 Plots for 4 Channels
        self.graphWidgets = [pg.PlotWidget() for _ in range(4)]
        colors = ['#FF4500', '#1E90FF', '#32CD32', '#FFD700']  # Orange, Blue, Green, Yellow for visibility on black
        self.curves = [graph.plot(pen=pg.mkPen(color=colors[i], width=2)) for i, graph in enumerate(self.graphWidgets)]
        for graph in self.graphWidgets:
            graph.setYRange(0, 255)  # Adjust Y range for uint8_t data
            graph.setBackground('k')  # Set black background
        
        # Create custom range sliders using two QSlider widgets
        self.start_slider = QtWidgets.QSlider(Qt.Orientation.Horizontal)
        self.start_slider.setInvertedAppearance(True)
        self.start_slider.setMinimum(0)
        self.start_slider.setMaximum(((BUFFER_SIZE - 12) // 4 - 1) // 2)  # Restrict start slider to at most half of the buffer size
        self.start_slider.setValue(self.start_slider.maximum())  # Start at the beginning of the buffer
        self.start_slider.setTickInterval(100)
        self.start_slider.setSingleStep(10)
        
        self.end_slider = QtWidgets.QSlider(Qt.Orientation.Horizontal)
        self.end_slider.setMinimum(((BUFFER_SIZE - 12) // 4 - 1) // 2)  # Start at half of the buffer size
        self.end_slider.setMaximum((BUFFER_SIZE - 12) // 4 - 1)
        self.end_slider.setValue((BUFFER_SIZE - 12) // 4 - 1)  # Set to the maximum value of the total buffer
        self.end_slider.setTickInterval(100)
        self.end_slider.setSingleStep(10)
        
        self.start_slider.valueChanged.connect(self.update_plot)
        self.end_slider.valueChanged.connect(self.update_plot)
        
        # Layout
        layout = QtWidgets.QVBoxLayout()
        for graph in self.graphWidgets:
            layout.addWidget(graph)
        
        # Horizontal layout for the sliders
        slider_layout = QtWidgets.QHBoxLayout()
        slider_layout.addWidget(QtWidgets.QLabel("Start"))
        slider_layout.addWidget(self.start_slider)
        
        slider_layout.addWidget(self.end_slider)
        slider_layout.addWidget(QtWidgets.QLabel("End"))  # Add "End" label to the right of the end slider
        layout.addLayout(slider_layout)
        
        layout.addWidget(self.start_button)
        layout.addWidget(self.stop_button)
        
        container = QtWidgets.QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)
        
        # Thread handling
        self.running = False
        self.thread = None
        self.channel_buffers = [np.zeros((BUFFER_SIZE - 12) // 4, dtype=np.uint8) for _ in range(4)]  # 4 buffers for 4 channels
        
        # Timer for UI updates
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(10)  # Update UI every 10ms
    
    def start_udp(self):
        if not self.running:
            self.running = True
            self.thread = threading.Thread(target=self.receive_udp, daemon=True)
            self.thread.start()

    def stop_udp(self):
        self.running = False

    def receive_udp(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((UDP_IP, UDP_PORT))
        sock.settimeout(1.0)  # Prevent indefinite blocking
        
        last_packet_count = None
        last_time = time.time()
        last_esp32_timestamp = None  # Store the last ESP32 timestamp

        with open(LOG_FILE, "w") as f:  # Open file for writing timestamps
            f.write("ESP32 Packet Count, ESP32 Timestamp (us), Time Since Last Packet (ms), ESP32 Time Diff (us)\n")  # Header
            while self.running:
                try:
                    data, _ = sock.recvfrom(BUFFER_SIZE)  # Expecting 1612 bytes
                    now = time.time()
                    delta = (now - last_time) * 1000  # Convert to milliseconds
                    last_time = now
                    
                    if len(data) >= 12:  # Ensure timestamp and packet count are present
                        packet_count_bytes = data[:4]
                        esp32_packet_count = int.from_bytes(packet_count_bytes, "little")  # Convert to int
                        timestamp_bytes = data[4:12]
                        esp32_timestamp = int.from_bytes(timestamp_bytes, "little")  # Convert to int
                        
                        # Check for packet loss
                        if last_packet_count is not None and esp32_packet_count != last_packet_count + 1:
                            print(f"⚠️ Packet Loss! Expected {last_packet_count + 1}, got {esp32_packet_count}")
                        last_packet_count = esp32_packet_count

                        # Calculate difference from the last ESP32 timestamp
                        esp32_delta = (esp32_timestamp - last_esp32_timestamp) if last_esp32_timestamp is not None else 0
                        last_esp32_timestamp = esp32_timestamp  # Update last ESP32 timestamp
                        
                        # Write timestamp, inter-packet delay, and ESP32 timestamp difference
                        f.write(f"{esp32_packet_count},{esp32_timestamp}, {delta:.2f}, {esp32_delta}\n")
                        f.flush()  # Ensure data is saved immediately
                        
                        # Extract waveform data in alternate mode (interleaved for 4 channels)
                        interleaved_data = np.frombuffer(data[12:], dtype=np.uint8)
                        num_samples = len(interleaved_data) // 4
                        channel_data = interleaved_data.reshape(num_samples, 4).T  # Transpose to get separate channels
                        for i in range(4):
                            self.channel_buffers[i] = channel_data[i]
                except socket.timeout:
                    continue  # Keep looping if no data received
                except Exception as e:
                    print("UDP Error:", e)
                    break  # Exit loop on other errors
        
        sock.close()
    
    def update_plot(self):
        """Update the graphs with a valid range from the start and end sliders, ensuring symmetry."""
        start = self.start_slider.maximum() - self.start_slider.value()
        end = self.end_slider.value()
        
        # Ensure valid range
        if start >= end:
            start, end = 0, min(len(self.channel_buffers[0]), self.end_slider.maximum())  # Reset to valid range if necessary
            return  # Do nothing if the range is invalid
        
        for i in range(4):
            x_values = np.arange(start, end)
            y_values = self.channel_buffers[i][start:end]
            self.curves[i].setData(x_values, y_values)  # Plot x and y values for the selected range  # Plot x and y values for the selected range  # Plot x and y values for the selected range  # Plot x and y values for the selected range  # Plot x and y values for the selected range  # Plot x and y values for the selected range  # Plot x and y values for the selected range

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    window = UDPPlotter()
    window.show()
    app.exec()
