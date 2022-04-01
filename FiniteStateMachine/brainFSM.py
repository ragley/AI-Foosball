from curses.ascii import TAB
import socket
import time
import asyncio
import math
from rodFSM import compute_next_state, compute_command
from Message import Message
from FSMConstants import *

def connect_to_server(host, port):
	while True:	
		try:
			sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			sock.connect((host, port))
			return sock
		except:
			print("Cannot find server... trying again...")		
			time.sleep(2)
			continue

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
    current_states = ["Block", "Block", "Block", "Block"]

    commands = {"robot_goal_rod_displacement_command":0, "robot_goal_rod_angle_command":0, "robot_two_rod_displacement_command":0, "robot_two_rod_angle_command":0, "robot_five_rod_displacement_command":0, "robot_five_rod_angle_command":0, "robot_three_rod_displacement_command":0, "robot_three_rod_angle_command":0}
    server_data = {"ball_x":0, "ball_y":0, "ball_Vx":0, "ball_Vy":0, "stop":False}

    try:
        host = LOCALHOST
        port = PORT       
        number_of_runs = 0
        total_time = 0.0
        sock = connect_to_server(host,port)

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
                speed = ball_speed(received.data["ball_Vx"], received.data["ball_Vy"])
                intercepts = compute_intercepts(GOAL_ROD["rodX"], TWO_ROD["rodX"], FIVE_ROD["rodX"], THREE_ROD["rodX"], TABLE["width"], received.data["ball_x"], received.data["ball_y"], received.data["ball_Vx"], received.data["ball_Vy"])

                output = await asyncio.gather(
                    compute_next_state(current_states[0],received.data["ball_x"], GOAL_ROD["rodX"], speed, received.data["stop"]),
                    compute_next_state(current_states[1],received.data["ball_x"], TWO_ROD["rodX"], speed, received.data["stop"]),
                    compute_next_state(current_states[2],received.data["ball_x"], FIVE_ROD["rodX"], speed, received.data["stop"]),
                    compute_next_state(current_states[3],received.data["ball_x"], THREE_ROD["rodX"], speed, received.data["stop"]),
                    compute_command(current_states[0], GOAL_ROD, received.data["ball_x"], received.data["ball_y"], TABLE, speed, intercepts[0]),
                    compute_command(current_states[1], TWO_ROD, received.data["ball_x"], received.data["ball_y"], TABLE, speed, intercepts[1]),
                    compute_command(current_states[2], FIVE_ROD, received.data["ball_x"], received.data["ball_y"], TABLE, speed, intercepts[2]),
                    compute_command(current_states[3], THREE_ROD, received.data["ball_x"], received.data["ball_y"], TABLE, speed, intercepts[3])
                )               

                k = 0
                for i in range(len(output)):
                    if i < len(current_states):
                        current_states[i] = output[i]
                    else:
                        for j in range(2):
                            if output[i][j] != -1:
                                commands[list(commands.keys())[k]] = int(output[i][j])
                            k+=1


                command_message = Message("POST")
                command_message.data.update(commands)
                sock.sendall(command_message.encode_to_send(True))

                total_time += time.perf_counter() - start
                number_of_runs += 1
                if number_of_runs % 100 == 0:
                    print(output)
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