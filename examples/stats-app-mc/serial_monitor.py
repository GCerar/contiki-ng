# ----------------------------------------------------------------------
# Monitor serial input from given port and store it into given file
#
# TODO: description
#
#
# ----------------------------------------------------------------------
import sys
import argparse
import serial
import os
import time
from datetime import datetime
from timeit import default_timer as timer

MAX_APP_TIME  = 1200

DEFAULT_FILE_NAME = "node_stats.txt"

BASEPORT = "/dev/ttyS"
BAUD = 460800           #460800
PARITY = serial.PARITY_NONE
STOPBIT = serial.STOPBITS_ONE
BYTESIZE = serial.EIGHTBITS

# ----------------------------------------------------------------------
# Monitor class
# ----------------------------------------------------------------------
class serial_monitor():

    def __init__(self):
        self.gotResponse = False

    
    def connect_to(self, p):
        try:
            self.port = "/dev/" + p
            self.ser = serial.Serial(self.port, BAUD, BYTESIZE, PARITY, STOPBIT, timeout=10)
            print("Serial monitor opened on port: " + self.port)
        except:
            print("Serial port not connected or in use!..Exiting now")
            sys.exit(1)


    def auto_connect(self):
        for i in range(2, 5):
            try:
                self.port = BASEPORT + str(i)
                self.ser = serial.Serial(self.port, BAUD, BYTESIZE, PARITY, STOPBIT, timeout=10)
                print("Serial monitor opened on port: " + self.port)
                break
            except:
                print("No serial port connected or all in use!..Exiting now")
                sys.exit(1)

    
    def read_line(self):
        value = self.ser.read_until(b'\n', None)
        return value


    def send_cmd(self, cmd):
        try:
            self.ser.write((cmd + "\n").encode("ASCII"))
        except:
            print("Error writing to device!")


    def wait_response(self, max_time):
        startTime = timer()
        while((timer() - startTime) < max_time):
            try:
                value = self.ser.readline()
                if not value:
                    break     
                if(chr(value[0]) == '>'):
                    self.gotResponse = True
                    break
            except KeyboardInterrupt:
                print("\n Keyboard interrupt!..Exiting now")
                sys.exit(1)


    def flush(self):
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

# ----------------------------------------------------------------------

    def prepare_file(self, filename):
        self.filename = filename
        self.file = open(filename, mode="w", encoding="ASCII")
        self.file.write(str(datetime.now())+"\n")
        self.file.write("----------------------------------------------------------------------------------------------- \n")
        self.file.write("Serial input from port:" + monitor.port + "\n")
        if(args.root):
            self.file.write("Device is root of the DAG network! \n")
        self.file.write("----------------------------------------------------------------------------------------------- \n")
        self.file.close()

    
    def store_to_file(self, data):
        self.file.write("[" + str(datetime.now().time())+"]: ")
        data = data.decode("ASCII")
        self.file.write(str(data))

    def store_str_to_file(self,string):
        self.file.write("[" + str(datetime.now().time())+"]: ")
        self.file.write(string)

    def rename_file(self, name):
        os.rename(DEFAULT_FILE_NAME, DEFAULT_FILE_NAME[:-4] + 
                  "_node_" + name + ".txt")
        print("File renamed to:" + DEFAULT_FILE_NAME[:-4] + 
                  "_node_" + name + ".txt")

# ----------------------------------------------------------------------

    def restart_vesna(self):
        print("Reset Vesna")
        # Export GPIO2_2 or linuxPin-66 to user space 
        try:
            os.system('echo 66 > /sys/class/gpio/export')
        except:
            print("Pin already exported")

        # Set the direction of the pin to output
        os.system('echo out > /sys/class/gpio/gpio66/direction')

        # Set the value to 0 - reset Vesna
        os.system('echo 0 > /sys/class/gpio/gpio66/value')

        time.sleep(5)

        # Set value back to 1 - wake Vesna up
        os.system('echo 1 > /sys/class/gpio/gpio66/value')

# ----------------------------------------------------------------------

    def close(self):
        self.ser.close()
        self.file.close()

monitor = serial_monitor()

# ----------------------------------------------------------------------
# Argument parser for selection output text file, port, root option,...
# ----------------------------------------------------------------------
parser = argparse.ArgumentParser(
    description="Store serial input into given file.",
    formatter_class=argparse.MetavarTypeHelpFormatter
)
parser.add_argument("-o", 
                    "--output", 
                    help="select file to store serial input", 
                    type=str,
                    required=False)
parser.add_argument("-p", 
                    "--port",   
                    help="""select serial port [ttyUSBx]...if no port 
                    given, program will find it automatically""",
                    type=str, 
                    required=False)
parser.add_argument("-r",
                    "--root",
                    help="set device as root of the network",
                    action="store_true")

args = parser.parse_args()

# ----------------------------------------------------------------------
# Open serial monitor
# ----------------------------------------------------------------------
if(not args.port):
    # Find port automatically - search for ttyUSB
    monitor.auto_connect()
else:
    # Connect to given port
    monitor.connect_to(args.port)

# ----------------------------------------------------------------------
# Prepare output file
# ----------------------------------------------------------------------
if(not args.output):
    name = DEFAULT_FILE_NAME
    print("Storing into default file: " + name)
else:
    name = args.output
    print("Storing into: " + name)

# (optional) Write first lines into it
monitor.prepare_file(name)

# Open file to append serial input to it
monitor.file = open(monitor.filename, "a")

# ----------------------------------------------------------------------
# Start the app
# ----------------------------------------------------------------------
print("Send start command")
monitor.send_cmd(">")

# Wait for response ('>' character) from Vesna for 3 seconds
print("Waiting for response...")
monitor.wait_response(3)

# If device is not responding, try again
if(not monitor.gotResponse):
    print("No response -> send start cmd again...")
    monitor.flush()
    monitor.send_cmd(">")
    monitor.wait_response(3)

if(not monitor.gotResponse):
    print("No response...please reset the device and try again")
    monitor.store_str_to_file("No response from Vesna...")
    monitor.close()
    sys.exit(1)

# ----------------------------------------------------------------------
# Set device as root of the network via serial CLI
# ----------------------------------------------------------------------
if(args.root):
    print("Set device as DAG root")
    monitor.send_cmd("*")
    
print("Start logging serial input:") 

# ----------------------------------------------------------------------
# Get general info about the app
# ----------------------------------------------------------------------

# Get max duration of the app ("AD 1200")
value = monitor.read_line()
if((chr(value[0]) == 'A') and (chr(value[1])== 'D')):
    MAX_APP_TIME = int(value[3:])

# ----------------------------------------------------------------------
# Read input lines while LINES_TO_READ or until app stops sending data
# ----------------------------------------------------------------------

line = 1
startTime = timer()
elapsedMin = 0
timeoutCnt = 0
timeoutGlobalCnt = 0

try:
    while(True):
        
        # Measure approximate elapsed time - just for a feeling (+- 10s)
        if((timer() - startTime) > 59):                                     # TODO: timeout is 20..test if this works   
            elapsedMin += 1                                                 # Doesnt work - fix it
            startTime = timer()
            #print(timer() - startTime)
            #print(timer() - startTime - 59)
            #startTime = timer() + (timer() - startTime - 59)

        # Failsafe mechanism - if Vesna for some reason stops responding 
        # So it didn't sent stop command 3min after MAX_APP_TIME, stop the monitor
        if elapsedMin > ((MAX_APP_TIME/60) + 2):
            print("\n \n Vesna must have crashed... :( \n \n")
            monitor.store_str_to_file(""" \n ERROR!
            Vesna has crashed durring application. 
            No stop command found 3min after end of application!""")
            break

        if timeoutCnt > 10 :
            print("\n \n Vesna must have crashed...Restart it now \n")
            monitor.store_str_to_file(""" \n WARNING!
            Vesna has crashed durring application. 
            10 Timeout event in a row happend. Restarting it now.. \n""")
            monitor.restart_vesna()
            # Restart stats-app
            time.sleep(1)
            print("Send start command")
            monitor.send_cmd(">")
            if(args.root):
                print("Set device as DAG root")
                monitor.send_cmd("*")
        
        # Read one line (until \n char)
        value = monitor.read_line()

        # Because of timeout setting, serial may return empty list
        if value:           
            # If stop command '=' found, exit monitor
            if(chr(value[0]) == '='):
                print("Found stop command (" + str(MAX_APP_TIME/60) +
                " minutes has elapsed)..stored " + str(line) + " lines.")
                break

            # Store value into file
            monitor.store_to_file(value)

            line += 1
            timeoutCnt = 0

        else:
            timeoutCnt += 1
            timeoutGlobalCnt += 1
            monitor.store_str_to_file(("Serial timeout occurred: " + str(timeoutGlobalCnt) + "\n"))
            print("Serial timeout occurred: " + str(timeoutCnt))

        # Update status line in terminal
        print("Line: " + str(line) + " (~ " + str(elapsedMin) + "|" + 
        str(int(MAX_APP_TIME/60)) + " min)", end="\r")
    
    print("")
    print("Done!..Exiting serial monitor")

except KeyboardInterrupt:
    print("\n Keyboard interrupt!..send stop command")
    monitor.send_cmd("=End")

    # Get last data ("driver statistics") before closing the monitor
    while(True):
        try:
            value = monitor.read_line()
            if(chr(value[0]) == '='):
                break
            else:
                monitor.store_to_file(value)
        except:
            print("Error closing monitor")  
            break
    print("Exiting serial monitor")


except serial.SerialException:
    print("Error opening port!..Exiting serial monitor")

except IOError:
    print("\n Serial port disconnected!.. Exiting serial monitor")

# ----------------------------------------------------------------------
# Close the monitor
# ----------------------------------------------------------------------
finally:
    monitor.close()
    # Rename a file with device ID
    #if(not args.output):
    #    monitor.rename_file(deviceID)