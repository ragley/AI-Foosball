import socket
import asyncio

from numpy import False_
from Message import Message
from rodFSM import compute_rod_linear

goal_rod = {"maxActuation":180, "playerSpacing":210, "rodX":1125, "numPlayers":3}
two_rod = {"maxActuation":352, "playerSpacing":245, "rodX":975, "numPlayers":2}
five_rod = {"maxActuation":112, "playerSpacing":120, "rodX":675, "numPlayers":5}
three_rod = {"maxActuation":224, "playerSpacing":185, "rodX":375, "numPlayers":3} #check measurements
table = {"robot_goalX":1200, "robot_goalY":350, "player_goalX":1200, "player_goalY":350, "goalWidth":200, "width":685, "length":1200}

commands = {"robot_goal_rod_displacement_command":0, "robot_goal_rod_angle_command":0, "robot_two_rod_displacement_command":0, "robot_two_rod_angle_command":0, "robot_five_rod_displacement_command":0, "robot_five_rod_angle_command":0, "robot_three_rod_displacement_command":0, "robot_three_rod_angle_command":0}
server_data = {"ballX": 0, "ballY": 0, "ball_vel_x":0, "ball_vel_y":0, "stop":False}

async def main():
    try:
        HOST = '127.0.0.1'  # The server's hostname or IP address
        PORT = 5000       # The port used by the server
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))

        while True:
            run = True
            message = Message("GET")
            message.data.update(server_data)
            sock.sendall(message.encode_to_send(True))
            recv_data = sock.recv(1024)
            received = Message("RECEIVED")
            received.decode_from_receive(recv_data)

            if "error" in received.data:
                print(received.data["error"])
                run = False
            else:
                for key in received.data:
                    if received.data[key] == "not found" and key != "action":
                        print("Server did not return all required data. MISSING: %s"%key)
                        run = False

            if run:

                output = await asyncio.gather(
                    compute_rod_linear(goal_rod, server_data["ballY"]),
                    compute_rod_linear(two_rod, server_data["ballY"]),
                    compute_rod_linear(five_rod, server_data["ballY"]),
                    compute_rod_linear(three_rod, server_data["ballY"])
                )
                print(output)

                
                for i in range(len(output)):
                    commands[list(commands.keys())[i*2]] = int(output[i])

            print(commands)
            command_message = Message("POST")
            command_message.data.update(commands)
            sock.sendall(command_message.encode_to_send(True))
    except KeyboardInterrupt:
        print("caught keyboard interrupt, exiting")
    finally:
        print("socket closed")
        sock.send(b'')
        sock.close()
if __name__ == '__main__':
    main()

asyncio.run(main())