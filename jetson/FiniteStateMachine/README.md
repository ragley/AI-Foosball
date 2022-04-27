Documentation for the Server portion of AI-Foosball Spring 2022 Senior Design Project.

# Finite State Machine

## Purpose

This module is responsible for determining commands for each rod based on the collected data from the camera and motor controllers. This data is received via the server 
and the resulting commands are sent back to the server in the format 

`robot_(rod descriptor)_rod_(displacement/angle)_(current/command)`

rod descriptors include (goal, 2, 5, 3) denoting which rod is referenced by the number of players

displacement/angle denotes which axis of the rod is referenced

8 of these commands are generated for each set of data that is collected. The FSM does not care about how old the data in the server is or if it has already computed 
commands on that data previously. The FSM computes faster than vision collects data or the CAN bus sends data so this method ensures that the most recent and accurate 
commands are always available on the server.

## Warning!!!
One must take special care in the design of the FSM or replacement for it. It is highly likely for the rods to either trap the ball between a player and the ground or up
against a wall. This can not be detected or prevented by the motors and they will attempt to torque through the ball and potentially damage the rod mechanism or cause 
gears to skip. Skipped gears require being manually reset and increase the liklihood of further damage to the table and lack of control of the rods. For this reason rods 
should also be zerod whenever they become visually off from their correct position. 

## States
A breif description of each of the FSM states. Each rod operates a copy of this FSM simultaneously allowing the rods to all move independently. 

### Stop
This state is triggered externally when motor controllers are not sending commands, either from an emergency stop being triggered, communication failure, or the game 
being paused. In this state the FSM continually output the previous command leaving the rods in their current location. 

### Idle
When the ball is a certain distance away the rods placed in the center of the table. This stops the rods from responding to values far from the rod and places them
in a central position to react as the ball approaches. 

### Open
This state is triggered when the ball is behind the rod. The players orient themselves parallel with the table to allow shots from behind to pass through 
undisturbed. The players still track the ball to place them in an optimal location after the ball passes underneath. 

### Block
This state is entered when the ball is in front of the rod. The 5 and 3 rod attempt to place a player directly in front of the ball to prevent forward progress. The 
goal and 2 rod place a player between the ball and the center of the goal, priortizing the defense of the goal.

### Prep
When the ball is close enough to the rod, it swings the players back and attempts to position itself offset from the ball slightly in order to kick the ball at an angle to 
approach the opposing goal. 

### Kick 
When the ball is closer to the rod. The rod waits until it has reached the location for kick before swinging the players forward. 

### Recover 
This is a series of four states that attempt to move the ball out of a dangerous positon. Such as underneath the rod after a failed kick or behind the goal rod. These 
positions are very likely for the ball to trapped and damage the mechanisms of the table. The rod first moves itself to a vertical position and then to the side of the
ball closer to the goal. The player than angles back and moves to the position of the ball. This should hit the ball from the side and push it to a better position to 
kick from. 

## FSM Constants

### Network
Variables related to connection with the server
- LOCALHOST: ip address of localhost
- PI_ADDRESS: ip address of the server on the raspberry pi
- PORT: port number of server on the raspberry pi

### State
variables related to the transition and execution of states, all distances in millimeters
- MOVEMENT_MARGIN: how close the rod must be to the desired location to transition in the recover states
- KICK_TIMEOUT: number of seconds before the kick state is forcibly exited
- LAST_POSITION: tells the FSM to resend the previous command
- PLAYER_LENGTH: length of player pad from center point
- NOISE_THRESHOLD: distance differences below this value are ignored to combat input noise
- MIN_VELOCITY_THRESHOLD: velocities below this number are stationary
- OPEN_PREP_RANGE: distance behind a rod where it enters the prep state
- BLOCK_PREP_RANGE: distance in front of a rod where it enters the prep state
- OPEN_KICK_RANGE: distance behind a rod where it enters the kick state
- BLOCK_KICK_RANGE = distance in front of a rod where it enters the kick state
- KICK_ANGLE: how far forward a rod swings when it kick
- PREP_ANGLE: how far back a rod swings when it is prepped
- BLOCK_ANGLE: angle for a blocking rod
- OPEN_ANGLE: angle for an open rod
- SPEED_THRESHOLD: speeds under this value will allow a kick attempt
- MIN_PLAYER_OFFSET: the closest a player can get to the near wall with the bumpers
- MAX_PLAYER_OFFSET: the closest a player can get to the far wall with the bumpers
- IDLE_RANGE: distance between a ball and rod before it idles 
- RECOVERY_LINEAR: distance between ball and player when recovering
- RECOVERY_ANGLE: angle rod takes when in recovery state

### Physical Dimensions
dimensions measured on the table, in millimeters
- GOAL_ROD: dimensions of the goal rod
- TWO_ROD: dimensions of the 2 rod
- FIVE_ROD: dimensions of the 5 rod
- THREE_ROD: dimensions of the 3 rod
- TABLE: dimensions of the table itself
