# Author: Phillip Jones
# Date: 10/30/2023
# Description: Client starter code that combines: 1) Simple GUI, 2) creation of a thread for
#              running the Client socket in parallel with the GUI, and 3) Simple recieven of mock sensor 
#              data for a server/cybot.for collecting data from the cybot.

# General Python tutorials (W3schools):  https://www.w3schools.com/python/

# Serial library:  https://pyserial.readthedocs.io/en/latest/shortintro.html 
import serial
import time # Time library   
# Socket library:  https://realpython.com/python-sockets/  
# See: Background, Socket API Overview, and TCP Sockets  
import socket
import tkinter as tk # Tkinter GUI library
# Thread library: https://www.geeksforgeeks.org/how-to-use-thread-in-tkinter-python/
import threading
import os  # import function for finding absolute path to this python script
import array
#Import/Include useful math and plotting functions
import numpy as np
import matplotlib.pyplot as plt


# angle_degrees: a vector (i.e., array of numbers) for which each element is an angle at which the sensor 
# makes a distance measurement.
# arange: used to to store into vector angle_degrees numbers from 0 degrees to 180 degrees, counting by 4's.
# Units: degrees


##### START Define Functions  #########

# Main: Mostly used for setting up, and starting the GUI
def main():

        global window  # Made global so quit function (send_quit) can access
        window = tk.Tk() # Create a Tk GUI Window

        # Last command label  
        global Last_command_Label  # Made global so that Client function (socket_thread) can modify
        Last_command_Label = tk.Label(text="Last Command Sent: ") # Creat a Label
        Last_command_Label.pack() # Pack the label into the window for display

        # Quit command Button
        quit_command_Button = tk.Button(text ="Press to Quit", command = send_quit)
        quit_command_Button.pack()  # Pack the button into the window for display

        # Cybot Scan command Button
        scan_command_Button = tk.Button(text ="Press to Scan", command = send_scan)
        scan_command_Button.pack() # Pack the button into the window for display

        f_command_Button = tk.Button(text ="Move Forward 10cm", command = send_F)
        f_command_Button.pack() # Pack the button into the window for display

        a_command_Button = tk.Button(text ="Turn Left 30 Degrees", command = send_A)
        a_command_Button.pack() # Pack the button into the window for display

        d_command_Button = tk.Button(text ="Turn Right 30 Degrees", command = send_D)
        d_command_Button.pack() # Pack the button into the window for display

        n_command_Button = tk.Button(text ="newline", command = send_n)
        n_command_Button.pack() # Pack the button into the window for display


        # Create a Thread that will run a fuction assocated with a user defined "target" function.
        # In this case, the target function is the Client socket code
        my_thread = threading.Thread(target=socket_thread) # Creat the thread
        my_thread.start() # Start the thread

        # Start event loop so the GUI can detect events such as button clicks, key presses, etc.
        window.mainloop()


# Quit Button action.  Tells the client to send a Quit request to the Cybot, and exit the GUI
def send_quit():
        global gui_send_message # Command that the GUI has requested be sent to the Cybot
        global window  # Main GUI window
        
        gui_send_message = "quit\n"   # Update the message for the Client to send
        time.sleep(1)
        window.destroy() # Exit GUI


# Scan Button action.  Tells the client to send a scan request to the Cybot
def send_scan():
        global gui_send_message # Command that the GUI has requested sent to the Cybot
        gui_send_message = "M\n"   # Update the message for the Client to send

def send_F():
        global gui_send_message # Command that the GUI has requested sent to the Cybot
        gui_send_message = "F\n"   # Update the message for the Client to send


def send_A():
        global gui_send_message # Command that the GUI has requested sent to the Cybot
        gui_send_message = "A\n"   # Update the message for the Client to send


def send_D():
        global gui_send_message # Command that the GUI has requested sent to the Cybot
        gui_send_message = "D\n"   # Update the message for the Client to send

def send_n():
        global gui_send_message # Command that the GUI has requested sent to the Cybot
        gui_send_message = "\n"   # Update the message for the Client to send


# Client socket code (Run by a thread created in main)
def socket_thread():
        # Define Globals
        global Last_command_Label # GUI label for displaying the last command sent to the Cybot
        global gui_send_message   # Command that the GUI has requested be sent to the Cybot

        # A little python magic to make it more convient for you to adjust where you want the data file to live
        # Link for more info: https://towardsthecloud.com/get-relative-path-python 
        absolute_path = os.path.dirname(__file__) # Absoult path to this python script
        relative_path = "./"   # Path to sensor data file relative to this python script (./ means data file is in the same directory as this python script)
        full_path = os.path.join(absolute_path, relative_path) # Full path to sensor data file
        filename = 'sensor-scan.txt' # Name of file you want to store sensor data from your sensor scan command

        # Choose to create either a UART or TCP port socket to communicate with Cybot (Not both!!)
        # UART BEGIN
        #cybot = serial.Serial('COM100', 115200)  # UART (Make sure you are using the correct COM port and Baud rate!!)
        # UART END

        # TCP Socket BEGIN (See Echo Client example): https://realpython.com/python-sockets/#echo-client-and-server
        HOST = "192.168.1.1"  # The server's hostname or IP address
        PORT = 288        # The port used by the server
        cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
        cybot_socket.connect((HOST, PORT))   # Connect to the socket  (Note: Server must first be running)
                      
        cybot = cybot_socket.makefile("rbw", buffering=0)  # makefile creates a file object out of a socket:  https://pythontic.com/modules/socket/makefile
        # TCP Socket END

        # Send some text: Either 1) Choose "Hello" or 2) have the user enter text to send
        send_message = "Hello\n"                            # 1) Hard code message to "Hello", or
        # send_message = input("Enter a message:") + '\n'   # 2) Have user enter text
        gui_send_message = "wait\n"  # Initialize GUI command message to wait                                

        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

        print("Sent to server: " + send_message) 

        # Send messges to server until user sends "quit"
        while send_message != 'quit\n':
                
                # Update the GUI to display command being sent to the CyBot
                command_display = "Last Command Sent:\t" + send_message
                Last_command_Label.config(text = command_display)  
               
                if (send_message == "M\n") or (send_message == "m\n"):
                        print("scanning")
                        distance = []


                        rx_message = cybot.readline()      # Wait for a message, readline expects message to end with "\n"

                        # while rx_message.decode() == '':
                        #         rx_message = cybot.readline()

                        # rx_message = cybot.readline()

                        k = 0
                        for x in range(0, 46):
                                # distance = int.from_bytes(rx_message[0:7], byteorder='big')
                                # print("measured value @ " + str(k) + " degrees = " + str(distance))
                                currentdistance = (int.from_bytes(rx_message, byteorder='big')-10)/256
                                print(str(currentdistance)+ " @ " + str(k) + " degrees")
                                rx_message = cybot.readline()
                                distance.append(currentdistance)
                                k += 4

                        # while len(distance) < 46:
                        #         distance.append(100)

                        # while len(distance) > 46:
                        #         distance.pop()

                        # angle_radians: a vector that contains converted elements of vector angle_degrees into radians 
                        # Units radians
                                
                        angle_degrees = np.arange(0,184,4)

                        angle_radians = (np.pi/180) * angle_degrees

                        # Create a polar plot
                        fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}) # One subplot of type polar
                        ax.plot(angle_radians, distance, color='r', linewidth=4.0)  # Plot distance verse angle (in radians), using red, line width 4.0
                        ax.set_xlabel('Distance (m)', fontsize = 14.0)  # Label x axis
                        ax.set_ylabel('Angle (degrees)', fontsize = 14.0) # Label y axis
                        ax.xaxis.set_label_coords(0.5, 0.15) # Modify location of x axis label (Typically do not need or want this)
                        ax.tick_params(axis='both', which='major', labelsize=14) # set font size of tick labels
                        ax.set_rmax(120)                    # Saturate distance at 2.5 meters
                        ax.set_rticks([0.5, 1, 1.5, 2, 2.5])   # Set plot "distance" tick marks at .5, 1, 1.5, 2, and 2.5 meters
                        ax.set_rlabel_position(-22.5)     # Adjust location of the radial labels
                        ax.set_thetamax(180)              # Saturate angle to 180 degrees
                        ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4)) # Set plot "angle" tick marks to pi/4 radians (i.e., displayed at 45 degree) increments
                                                                # Note: added .1 to pi to go just beyond pi (i.e., 180 degrees) so that 180 degrees is displayed
                        ax.grid(True)                     # Show grid lines
                        # Create title for plot (font size = 14pt, y & pad controls title vertical location)
                        ax.set_title("Mock-up Polar Plot of CyBot Sensor Scan from 0 to 180 Degrees", size=14, y=1.0, pad=-24) 
                        plt.show()  # Display plot
                        print("done")
                
                elif (send_message == "A\n"):
                        print("turning left")

                elif (send_message == "D\n"):
                        print("turning right")

                elif (send_message == "F\n"):
                        print("moving forward")

                else:
                        print("Waiting for server reply\n")
                        rx_message = cybot.readline()      # Wait for a message, readline expects message to end with "\n"
                        print("Got a message from server: " + rx_message.decode() + "\n") # Convert message from bytes to String (i.e., decode)


                # Choose either: 1) Idle wait, or 2) Request a periodic status update from the Cybot
                # 1) Idle wait: for gui_send_message to be updated by the GUI
                while gui_send_message == "wait\n": 
                        time.sleep(.1)  # Sleep for .1 seconds
                send_message = gui_send_message

                # 2) Request a periodic Status update from the Cybot:
                # every .1 seconds if GUI has not requested to send a new command
                #time.sleep(.1)
                #if(gui_send_message == "wait\n"):   # GUI has not requested a new command
                #        send_message = "status\n"   # Request a status update from the Cybot
                #else:
                #        send_message = gui_send_message  # GUI has requested a new command

                gui_send_message = "wait\n"  # Reset gui command message request to wait                        

                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                
        print("Client exiting, and closing file descriptor, and/or network socket\n")
        time.sleep(2) # Sleep for 2 seconds
        cybot.close() # Close file object associated with the socket or UART
        cybot_socket.close()  # Close the socket (NOTE: comment out if using UART interface, only use for network socket option)

##### END Define Functions  #########


### Run main ###
main()