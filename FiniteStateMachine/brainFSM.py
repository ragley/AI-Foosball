import socket
import time
import asyncio
import math
from numpy import true_divide
from rodFSM import compute_next_state, compute_command
from Message import Message

def compute_intercepts(goalX, twoX, fiveX, threeX, width, ballX, ballY, ball_vel_x, ball_vel_y):
    intercepts = [-1, -1, -1, -1]
    slope = ball_vel_y / ball_vel_x
    b = ballY - slope * ballX
    if ball_vel_x > 0:
        direction = True
    else:
        direction = False
    
    if (direction and ballX < goalX) or (not direction and ballX > goalX):
        intercept = slope * ballX + b
        if 0 <= intercept <= width:
            intercepts[0] = intercept
    if (direction and ballX < twoX) or (not direction and ballX > twoX):
        intercept = slope * ballX + b
        if 0 <= intercept <= width:
            intercepts[1] = intercept
    if (direction and ballX < fiveX) or (not direction and ballX > fiveX):
        intercept = slope * ballX + b
        if 0 <= intercept <= width:
            intercepts[2] = intercept
    if (direction and ballX < threeX) or (not direction and ballX > threeX):
        intercept = slope * ballX + b
        if 0 <= intercept <= width:
            intercepts[3] = intercept
    
    return intercepts

def ball_speed(ball_vel_x, ball_vel_y):
    return math.sqrt(ball_vel_x**2 + ball_vel_y**2)


async def main():
    goal_rod = {"maxActuation":180, "playerSpacing":210, "rodX":1125, "numPlayers":3}
    two_rod = {"maxActuation":352, "playerSpacing":245, "rodX":975, "numPlayers":2}
    five_rod = {"maxActuation":112, "playerSpacing":120, "rodX":675, "numPlayers":5}
    three_rod = {"maxActuation":224, "playerSpacing":185, "rodX":375, "numPlayers":3} #check measurements
    table = {"robot_goalX":1200, "robot_goalY":350, "player_goalX":1200, "player_goalY":350, "goalWidth":200, "width":685, "length":1200}
    current_states = ["Block", "Block", "Block", "Block"]

    commands = {"robot_goal_rod_displacement_command":0, "robot_goal_rod_angle_command":0, "robot_two_rod_displacement_command":0, "robot_two_rod_angle_command":0, "robot_five_rod_displacement_command":0, "robot_five_rod_angle_command":0, "robot_three_rod_displacement_command":0, "robot_three_rod_angle_command":0}
    server_data = {"ballX": 0, "ballY": 0, "ball_vel_x":0, "ball_vel_y":0, "stop":False}

    try:
        HOST = '127.0.0.1'  # The server's hostname or IP address
        PORT = 5000       # The port used by the server
        number_of_runs = 0
        total_time = 0.0
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))

        while True:
            run = True 
            message = Message("GET")
            message.data.update(server_data)
            start = time.perf_counter()
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
                speed = ball_speed(server_data["ball_vel_x"], server_data["ball_vel_y"])
                intercepts = compute_intercepts(goal_rod["rodX"], two_rod["rodX"], five_rod["rodX"], three_rod["rodX"], table["width"], server_data["ballX"], server_data["ballY"], server_data["ball_vel_x"], server_data["ball_vel_y"])

                output = await asyncio.gather(
                    compute_next_state(current_states[0],server_data["ballX"], goal_rod["rodX"], speed, server_data["stop"]),
                    compute_next_state(current_states[1],server_data["ballX"], two_rod["rodX"], speed, server_data["stop"]),
                    compute_next_state(current_states[2],server_data["ballX"], five_rod["rodX"], speed, server_data["stop"]),
                    compute_next_state(current_states[3],server_data["ballX"], three_rod["rodX"], speed, server_data["stop"]),
                    compute_command(current_states[0], goal_rod, server_data["ballX"], server_data["ballY"], table, speed, intercepts[0]),
                    compute_command(current_states[1], two_rod, server_data["ballX"], server_data["ballY"], table, speed, intercepts[1]),
                    compute_command(current_states[2], five_rod, server_data["ballX"], server_data["ballY"], table, speed, intercepts[2]),
                    compute_command(current_states[3], three_rod, server_data["ballX"], server_data["ballY"], table, speed, intercepts[3])
                )
                print(output)

                k = 0
                for i in range(len(output)):
                    if i < len(current_states):
                        current_states[i] = output[i]
                    else:
                        for j in range(2):
                            if output[i][j] != -1:
                                commands[list(commands.keys())[k]] = int(output[i][j])
                            k+=1

            print(commands)
            command_message = Message("POST")
            command_message.data.update(commands)
            sock.sendall(command_message.encode_to_send(True))

            total_time += time.perf_counter() - start
            number_of_runs += 1
            print(total_time / number_of_runs)
    except KeyboardInterrupt:
        print("caught keyboard interrupt, exiting")
    finally:
        print("socket closed")
        sock.send(b'')
        sock.close()
if __name__ == '__main__':
    main()

asyncio.run(main())