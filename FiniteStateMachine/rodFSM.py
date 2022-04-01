import math
from FSMConstants import *

async def compute_next_state(currentState, ballX, rodX, ballSpeed, stop):
    if stop:
        return "Stop"
    elif abs(ballX - rodX) > IDLE_RANGE:
        return "Idle"
    elif ballX > rodX:
        return "Open"
    elif ballX < rodX or (currentState == "Kick" and abs(ballX - rodX) > KICK_RANGE) or (currentState == "Stop" and not stop):
        return "Block"
    elif (currentState == "Open" and ballSpeed < SPEED_THRESHOLD and ballX - rodX < PREP_RANGE) or (currentState == "Block" and ballSpeed < SPEED_THRESHOLD and rodX - ballX < PREP_RANGE):
        return "Prep"
    elif currentState == "Prep" and abs(ballX - rodX) < KICK_RANGE:
        return "Kick"

async def compute_command(currentState, rod, ballX, ballY, table, speed, intercept):
    if currentState == "Stop":
        return state_stop()
    elif currentState == "Idle":
        return state_idle(rod)
    elif currentState == "Open":
        return state_open(rod, ballY, speed, intercept)
    elif currentState == "Block":
        return state_block(rod, ballX, ballY, table["robot_goalX"], table["robot_goalY"], speed, intercept)
    elif currentState == "Prep":
        return state_prep(rod, ballX, ballY, table["player_goalX"], table["player_goalY"], speed, intercept)
    elif currentState == "Kick":
        return state_kick()
    else:
        return [-1, -1]
    

def compute_rod_linear(rod, desiredY):
    maxActuation = rod["maxActuation"]
    playerSpacing = rod["playerSpacing"]
    if desiredY < MIN_PLAYER_OFFSET:
        return 0
    if desiredY > MAX_PLAYER_OFFSET:
        return maxActuation

    if playerSpacing > maxActuation:
        actuation = (desiredY-MIN_PLAYER_OFFSET) % playerSpacing
    else:
        overlap = maxActuation - playerSpacing
        adjusted_actuation = maxActuation - int(overlap / 2)
        player_offset =  int((desiredY-MIN_PLAYER_OFFSET) / adjusted_actuation)
        actuation = ((desiredY-MIN_PLAYER_OFFSET) % adjusted_actuation) + int(overlap * player_offset / 2)

    if actuation > maxActuation:
        return 0
    else:
        return actuation

def state_stop():
    return [-1, -1]

def state_idle(rod):
    maxActuation = rod["maxActuation"]
    return [maxActuation / 2, 0]

def state_open(rod, ballY, speed, intercept):
    if speed < MIN_VELOCITY_THRESHOLD or intercept == -1:
        desiredY = ballY 
    else:
        desiredY = intercept 
    return [compute_rod_linear(rod, desiredY), -90]

def state_block(rod, ballX, ballY, goalX, goalY, speed, intercept):    
    if speed > MIN_VELOCITY_THRESHOLD and intercept != -1:
        desiredY = intercept
    else:
        slope = (goalY - ballY) / (goalX - ballX)
        b = ballY - slope * ballX
        shot_intercept = slope * ballX + b
        desiredY = shot_intercept

    return [compute_rod_linear(rod, desiredY), 0]

def state_prep(rod, ballX, ballY, goalX, goalY, speed, intercept):
    deltaY = 17.5 * math.degrees(math.sin(math.atan( (goalY - ballY) / (goalX - ballX) )))
    if speed < MIN_VELOCITY_THRESHOLD or intercept == -1:
        desiredY = ballY + deltaY
    else:
        desiredY = intercept + deltaY
    return [compute_rod_linear(rod, desiredY), -45]

def state_kick():
    return [-1, 45]


# print("goal rod")
# print(compute_rod_linear(goal_rod, 150))
# print(compute_rod_linear(goal_rod, 300))
# print(compute_rod_linear(goal_rod, 450))
# print(compute_rod_linear(goal_rod, 600))
# print("")

# print("2 rod")
# print(compute_rod_linear(two_rod, 150))
# print(compute_rod_linear(two_rod, 300))
# #print(compute_rod_linear(two_rod, 350))
# #print(compute_rod_linear(two_rod, 360))
# print(compute_rod_linear(two_rod, 450))
# print(compute_rod_linear(two_rod, 600))
# print("")

# print("five rod")
# print(compute_rod_linear(five_rod, 150))
# print(compute_rod_linear(five_rod, 300))
# print(compute_rod_linear(five_rod, 450))
# print(compute_rod_linear(five_rod, 600))
# print("")

# print("3 rod")
# print(compute_rod_linear(three_rod, 150))
# print(compute_rod_linear(three_rod, 300))
# print(compute_rod_linear(three_rod, 450))
# print(compute_rod_linear(three_rod, 600))