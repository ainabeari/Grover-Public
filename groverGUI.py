from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QPushButton, QWidget, QGridLayout
from PyQt5.QtCore import QSize, Qt, QThread, pyqtSignal, QTimer
from PyQt5.QtGui import QPalette, QColor, QFont
import sys
import serial
import time

class SerialThread(QThread):
    data_received = pyqtSignal(float, float, str, float)

    def __init__(self, arduino, parent=None):
        super(SerialThread, self).__init__(parent)
        self.arduino = arduino
        # self.timer = QTimer()
        # self.timer.timeout.connect(self.run)
        # self.timer.start(1000) # Update GUI every second

    def run(self):
        while(True):
            if self.arduino.in_waiting > 0: 
                data = self.arduino.readline().decode().rstrip().split(',')
                if len(data) == 3:
                    temperature, humidity, fire = map(float, data)
                    # Parameter to determine if there is a fire or not. Adjust percentage as needed
                    if temperature < 60:
                        gas = "Fresh Air"
                    else:
                        gas = "Wildfire Risk!"
                    self.data_received.emit(temperature, humidity, gas, fire*100)

# Subclass QMainWindow to customize your application's main window
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.arduino = serial.Serial('COM8', 115200)
        self.serial_thread = SerialThread(self.arduino)
        self.serial_thread.data_received.connect(self.update_values)
        self.serial_thread.start()

        # Set the window properties (title and initial size)
        self.setWindowTitle("Grover Main Controls")
        self.setGeometry(100, 100, 2500, 1000)  # (x, y, width, height)

        # Create widgets (QLabel and QPushButton)
        tempLabel = QLabel("Temperature")
        tempLabel.setFont(QFont("Arial", 22, QFont.Bold))
        self.tempValue = QLabel("Temperature Here")
        self.tempValue.setFont(QFont("Arial", 18, QFont.Normal))

        humidLabel = QLabel("Humidity")
        humidLabel.setFont(QFont("Arial", 22, QFont.Bold))
        self.humidValue = QLabel("Humidity Here")
        self.humidValue.setFont(QFont("Arial", 18, QFont.Normal))

        gasLabel = QLabel("Air Quality")
        gasLabel.setFont(QFont("Arial", 22, QFont.Bold))
        self.gasName = QLabel("Gas Detected")
        self.gasName.setFont(QFont("Arial", 18, QFont.Normal))

        fireLabel = QLabel("Percentage Risk")
        fireLabel.setFont(QFont("Arial", 22, QFont.Bold))
        self.fireRisk = QLabel("Percentage")
        self.fireRisk.setFont(QFont("Arial", 18, QFont.Normal))

        dataReq = QPushButton("Request Data")
        dataReq.clicked.connect(self.isDataClicked)
        movePlate = QPushButton("Move Sensor Plate")
        movePlate.clicked.connect(self.isClicked)
        # senseDOWN = QPushButton("Lower Sensor Plate")
        # senseDOWN.clicked.connect(self.isDownClicked)

        # Create a central widget for the main window
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        self.setStyleSheet("background-color: palegreen")
        # Create vertical and horizontal layouts
        grid_layout = QGridLayout()
        # Set the layout for the central widget
        central_widget.setLayout(grid_layout)

        # Add widgets to layouts
        grid_layout.addWidget(tempLabel, 0, 0)
        grid_layout.addWidget(self.tempValue, 1, 0)
        grid_layout.addWidget(humidLabel, 0, 1)
        grid_layout.addWidget(self.humidValue, 1, 1)
        grid_layout.addWidget(gasLabel, 0, 2)
        grid_layout.addWidget(self.gasName, 1, 2)
        grid_layout.addWidget(fireLabel, 0, 3)
        grid_layout.addWidget(self.fireRisk, 1, 3)
        grid_layout.addWidget(dataReq, 2, 0)
        grid_layout.addWidget(movePlate, 2, 1)
        # grid_layout.addWidget(senseDOWN, 2, 2)

    def sendArduino(self, command):
        self.arduino.write(command.encode('utf-8'))

    def isDataClicked(self):
        print("Data Requested")
        # if self.arduino.in_waiting > 0: 
        #     data = self.arduino.readline().decode().rstrip().split(',')
        #     if len(data) == 3:
        #         temperature, humidity, fire = map(str, data)
        #         # Parameter to determine if there is a fire or not. Adjust percentage as needed
        #         if float(fire) < 50:
        #             gas = "Fresh Air"
        #         else:
        #             gas = "Wildfire Risk"
        #         self.data_received.emit(temperature, humidity, gas, fire)

    def isClicked(self):
        print("Stepper Motor Signal Sent")
        self.sendArduino('T')
        self.sendArduino('T')
        self.sendArduino('T')
        self.sendArduino('T')
        self.sendArduino('T')

    def update_values(self, temperature, humidity, gas, fire):
        self.tempValue.setText(f"{temperature} °C")
        self.humidValue.setText(f"{humidity} %")
        self.gasName.setText(gas)
        self.fireRisk.setText(f"{fire} %")

# Create a PyQt application
app = QApplication(sys.argv)
window = MainWindow()
# Show the Window
window.show()
# Run the application's event loop
app.exec()