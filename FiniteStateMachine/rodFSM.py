import math

THRESHOLD = 0.1

async def compute_next_state(currentState, ballX, rodX, ballSpeed, stop):
    if stop:
        return "Stop"
    elif abs(ballX - rodX) > 600:
        return "Idle"
    elif ballX > rodX:
        return "Open"
    elif ballX < rodX or currentState == "Kick" or (currentState == "Stop" and not stop):
        return "Block"
    elif (currentState == "Open" and ballSpeed < 1000 and ballX - rodX < 100) or (currentState == "Block" and ballSpeed < 1000 and rodX - ballX < 100):
        return "Prep"
    elif currentState == "Prep" and abs(ballX - rodX) < 40:
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
    if desiredY < 45:
        return 0
    if desiredY > 640:
        return maxActuation

    if playerSpacing > maxActuation:
        actuation = (desiredY-45) % playerSpacing
    else:
        overlap = maxActuation - playerSpacing
        adjusted_actuation = maxActuation - int(overlap / 2)
        player_offset =  int((desiredY-45) / adjusted_actuation)
        actuation = ((desiredY-45) % adjusted_actuation) + int(overlap * player_offset / 2)

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
    if speed < THRESHOLD or intercept == -1:
        desiredY = ballY 
    else:
        desiredY = intercept 
    return [compute_rod_linear(rod, desiredY), -90]

def state_block(rod, ballX, ballY, goalX, goalY, speed, intercept):    
    if speed > THRESHOLD:
        desiredY = intercept
    else:
        slope = (goalY - ballY) / (goalX - ballX)
        b = ballY - slope * ballX
        shot_intercept = slope * ballX + b
        desiredY = shot_intercept

    return [compute_rod_linear(rod, desiredY), 0]

def state_prep(rod, ballX, ballY, goalX, goalY, speed, intercept):
    deltaY = 17.5 * math.degrees(math.sin(math.atan( (goalY - ballY) / (goalX - ballX) )))
    if speed < THRESHOLD or intercept == -1:
        desiredY = ballY + deltaY
    else:
        desiredY = intercept + deltaY
    return [compute_rod_linear(rod, desiredY), -45]

def state_kick():
    return [-1, 45]


goal_rod = {"maxActuation":180, "playerSpacing":210, "rodX":1125, "numPlayers":3}
two_rod = {"maxActuation":352, "playerSpacing":245, "rodX":975, "numPlayers":2}
five_rod = {"maxActuation":112, "playerSpacing":120, "rodX":675, "numPlayers":5}
three_rod = {"maxActuation":224, "playerSpacing":185, "rodX":375, "numPlayers":3} #check measurements
goal = {"goalX":1200, "goalY":350, "goalWidth":200}


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