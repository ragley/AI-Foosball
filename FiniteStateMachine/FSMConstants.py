#NETWORK 
LOCALHOST = "127.0.0.1"
PI_ADDRESS = "192.168.0.1"
PORT = 5000

#STATE 
MIN_VELOCITY_THRESHOLD = 0.1
PREP_RANGE = 100
KICK_RANGE = 50
SPEED_THRESHOLD = 1000
MIN_PLAYER_OFFSET = 45
MAX_PLAYER_OFFSET = 640
IDLE_RANGE = 600

#PHYSICAL DIMENSIONS
GOAL_ROD = {"maxActuation":180, "playerSpacing":210, "rodX":1125, "numPlayers":3}
TWO_ROD = {"maxActuation":352, "playerSpacing":245, "rodX":975, "numPlayers":2}
FIVE_ROD = {"maxActuation":112, "playerSpacing":120, "rodX":675, "numPlayers":5}
THREE_ROD = {"maxActuation":224, "playerSpacing":185, "rodX":375, "numPlayers":3}
TABLE = {"robot_goalX":1200, "robot_goalY":350, "player_goalX":1200, "player_goalY":350, "goalWidth":200, "width":685, "length":1200}