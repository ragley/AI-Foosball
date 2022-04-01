import socket
import asyncio

from Message import Message
from rodFSM import compute_rod_linear
from FSMConstants import *

async def main():
    commands = {"robot_goal_rod_displacement_command":0, "robot_goal_rod_angle_command":0, "robot_two_rod_displacement_command":0, "robot_two_rod_angle_command":0, "robot_five_rod_displacement_command":0, "robot_five_rod_angle_command":0, "robot_three_rod_displacement_command":0, "robot_three_rod_angle_command":0}
    server_data = {"ball_x":0, "ball_y":0, "ball_Vx":0, "ball_Vy":0, "stop":False}

    try:
        host = LOCALHOST
        port = PORT       
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))

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
                    compute_rod_linear(GOAL_ROD, received.data["ball_y"]),
                    compute_rod_linear(TWO_ROD, received.data["ball_y"]),
                    compute_rod_linear(FIVE_ROD, received.data["ball_y"]),
                    compute_rod_linear(THREE_ROD, received.data["ball_y"])
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