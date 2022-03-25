import os
import socket
import can
import time
import threading
import struct
from Message import Message


#Closes and opens the CAN communication
os.system('sudo ifconfig can0 down')
os.system('sudo ip link set can0 type can bitrate 500000')
os.system('sudo ifconfig can0 up')

#Initilizes the CAN
can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan')


#Initilizes and connects to the socket
#host = "192.168.0.1"
host = '127.0.0.1'
port = 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))

sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock2.connect((host, port))

#Hex values of all binary values 
#41 42 44 48
#81 82 84 88
#b0 20
#Dictionary for CAN
Controller = {
    0b01001000: {"Name": "Zero_goal"},
    0b01000100: {"Name": "Zero_2"},
    0b01000010: {"Name": "Zero_5"},
    0b01000001: {"Name": "Zero_3"},
    0b10011000: {"Name": "Controller_goal"},
    0b10010100: {"Name": "Controller_2"},
    0b10010010: {"Name": "Controller_5"},
    0b10010001: {"Name": "Controller_3"},
    0b10001000: {"Name": "Controller_goal_stop"},
    0b10000100: {"Name": "Controller_2_stop"},
    0b10000010: {"Name": "Controller_5_stop"},
    0b10000001: {"Name": "Controller_3_stop"},
    0b10110000: {"Name": "Goal_update"},
    0b00100000: {"Name": "Goal_reset"},
}

def receiveCan():
    #update flag for the server
    update = False
    stop = True
    goal_counter_player = 0
    goal_counter_robot = 0
    #Dictionary for the server, current position
    rods_current = {}
    
    while True:
        #Received message with 0 second timeout
        msg1 = can0.recv(0.0)
        ##print(msg1)

        if msg1 is not None:
            id = msg1.arbitration_id
            update = True
            if id in Controller.keys():
                #print(msg1)
                name = Controller[msg1.arbitration_id]["Name"]
                split1 = msg1.data[0:4]
                split2 = msg1.data[4:8]
                data1 = struct.unpack('>f',split1)
                data2 = struct.unpack('>f',split2)
                #Non-stop messages
                if name == "Controller_goal":
                    rods_current["robot_goal_rod_displacement_current"] = data1
                    rods_current["robot_goal_rod_angle_current"] = data2
                    stop = False
                elif name == "Controller_2":
                    rods_current["robot_2_rod_displacement_current"] = data1
                    rods_current["robot_2_rod_angle_current"] = data2
                    stop = False
                elif name == "Controller_5":
                    rods_current["robot_5_rod_displacement_current"] = data1
                    rods_current["robot_5_rod_angle_current"] = data2
                    stop = False
                elif name == "Controller_3":
                    rods_current["robot_3_rod_displacement_current"] = data1
                    rods_current["robot_3_rod_angle_current"] = data2
                    rods_current["stop"] = False
                    stop = False
                #Stop messages    
                elif name == "Controller_goal_stop":
                    rods_current["robot_goal_rod_displacement_current"] = data1
                    rods_current["robot_goal_rod_angle_current"] = data2
                    stop = True
                elif name == "Controller_2_stop":
                    rods_current["robot_2_rod_displacement_current"] = data1
                    rods_current["robot_2_rod_angle_current"] = data2
                    stop = True
                elif name == "Controller_5_stop":
                    rods_current["robot_5_rod_displacement_current"] = data1
                    rods_current["robot_5_rod_angle_current"] = data2
                    stop = True
                elif name == "Controller_3_stop":
                    rods_current["robot_3_rod_displacement_current"] = data1
                    rods_current["robot_3_rod_angle_current"] = data2
                    stop = True
                elif name == "Goal_update":
                    goal_counter_player = data1
                    goal_counter_robot = data2
                    update_goal = True
                
                rods_current["score_player"] = goal_counter_player
                rods_current["score_robot"] = goal_counter_robot
                rods_current["stop"] = stop
                
                ##print("Message exist")
                #print(Controller[msg1.arbitration_id]["Name"])
                #print(data1)
                #print(data2)
            else:
                #Tells server an unexpected message arrived
                rods_current[msg1.arbitration_id] = str(msg1)
                #print(msg1)
        if update:        
            message = Message("POST")
            message.data.update(rods_current)
            sock.sendall(message.encode_to_send(True))
            rods_current={}
            update = False
            #print(message.data)

def sendCan():
    #sets a timer to send CAN messages every 20 miliseconds
    timer = time.perf_counter() * 1000
    reset_flag = False
    delay = 10
    
    rods_desired = {
    "robot_goal_rod_displacement_command": 0,
    "robot_goal_rod_angle_command": 0,
    "robot_2_rod_displacement_command": 0,
    "robot_2_rod_angle_command": 0,
    "robot_5_rod_displacement_command": 0,
    "robot_5_rod_angle_command": 0,
    "robot_3_rod_displacement_command": 0,
    "robot_3_rod_angle_command": 0,
    }
    while True:
        run = True
        message = Message("GET")
        message.data.update(rods_desired)
        sock2.sendall(message.encode_to_send(True))
        recv_data = sock2.recv(1024)
        received = Message("RECEIVED")
        received.decode_from_receive(recv_data)
        #print(received.data)
        
        if "error" in received.data:
                print(received.data["error"])
                run = False
        else:
            for key in received.data:
                if received.data[key] == "not found" and key != "action":
                    print("Server did not return all required data. MISSING: %s"%key)
                    run = False
        
        if run:
            if(time.perf_counter()*1000 - timer > delay):
                print(time.perf_counter()*1000 - timer)
                timer = time.perf_counter()*1000
                goal_data1 = bytearray(struct.pack('>f',received.data["robot_goal_rod_displacement_command"]))
                goal_data2 = bytearray(struct.pack('>f',received.data["robot_goal_rod_angle_command"]))
                rod2_data1 = bytearray(struct.pack('>f',received.data["robot_2_rod_displacement_command"]))
                rod2_data2 = bytearray(struct.pack('>f',received.data["robot_2_rod_angle_command"]))
                rod5_data1 = bytearray(struct.pack('>f',received.data["robot_5_rod_displacement_command"]))
                rod5_data2 = bytearray(struct.pack('>f',received.data["robot_5_rod_angle_command"]))
                rod3_data1 = bytearray(struct.pack('>f',received.data["robot_3_rod_displacement_command"]))
                rod3_data2 = bytearray(struct.pack('>f',received.data["robot_3_rod_angle_command"]))
                
                msg_goal = can.Message(arbitration_id = 0b00011000, data=goal_data1 + goal_data2, is_extended_id=False)
                msg_2 = can.Message(arbitration_id = 0b00010100, data=rod2_data1 + rod2_data2, is_extended_id=False)
                msg_5 = can.Message(arbitration_id = 0b00010010, data=rod5_data1 + rod5_data2, is_extended_id=False)
                msg_3 = can.Message(arbitration_id = 0b00010001, data=rod3_data1 + rod3_data2, is_extended_id=False)
                can0.send(msg_goal)
                can0.send(msg_2)
                can0.send(msg_5)
                can0.send(msg_3)
                ##print ('messages sent')

        if reset_flag:
            msg_game_reset = can.Message(arbitration_id = 0b00011111, data=0, is_extended_id=False)
            msg_goal_reset = can.Message(arbitration_id = 0b00100000, data=0, is_extended_id=False)
            can0.send(msg_reset)
            reset_flag = False

                

try:
    t1 = threading.Thread(target=receiveCan)
    t2 = threading.Thread(target=sendCan)
    t1.start()
    t2.start()
    while True:
        t1.join()
        t2.join()
except KeyboardInterrupt:
    print("Exiting")
finally:
    sock.send(b'')
    sock.close()
    sock2.send(b'')
    sock2.close()
    os.system('sudo ifconfig can0 down')

