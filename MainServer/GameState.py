class GameState:
    _robot_goal_rod_linear = 0
    _robot_goal_rod_rotation = 0
    _robot_2_rod_linear = 0
    _robot_2_rod_rotation = 0
    _robot_5_rod_linear = 0
    _robot_5_rod_rotation = 0
    _robot_3_rod_linear = 0
    _robot_3_rod_rotation = 0

    _human_goal_rod_linear = 0
    _human_goal_rod_rotation = 0
    _human_3_rod_linear = 0
    _human_3_rod_rotation = 0
    _human_5_rod_linear = 0
    _human_5_rod_rotation = 0
    _human_3_rod_linear = 0
    _human_3_rod_rotation = 0

    _ball_position = [0, 0]
    _ball_velocity = [0, 0]
    _ball_prediction_position = [0, 0, 0, 0]
    _ball_prediction_time = [0, 0, 0, 0]

    _game_score = [0, 0]
    _game_time = 0

    def __init__(self):
        pass

    def update_rods(self, vals):
        self._robot_goal_rod_linear = vals[0]
        self._robot_goal_rod_rotation = vals[1]
        self._robot_2_rod_linear = vals[2]
        self._robot_2_rod_rotation = vals[3]
        self._robot_5_rod_linear = vals[4]
        self._robot_5_rod_rotation = vals[5]
        self._robot_3_rod_linear = vals[6]
        self._robot_3_rod_rotation = vals[7]
        self._human_goal_rod_linear = vals[8]
        self._human_goal_rod_rotation = vals[9]
        self._human_2_rod_linear = vals[10]
        self._human_2_rod_rotation = vals[11]
        self._human_5_rod_linear = vals[12]
        self._human_5_rod_rotation = vals[13]
        self._human_3_rod_linear = vals[14]
        self._human_3_rod_rotation = vals[15]

    def get_rods(self):
        vals = []
        vals.append(self._robot_goal_rod_linear)
        vals.append(self._robot_goal_rod_rotation)
        vals.append(self._robot_2_rod_linear)
        vals.append(self._robot_2_rod_rotation)
        vals.append(self._robot_5_rod_linear)
        vals.append(self._robot_5_rod_rotation)
        vals.append(self._robot_3_rod_linear)
        vals.append(self._robot_3_rod_rotation)
        vals.append(self._human_goal_rod_linear)
        vals.append(self._human_goal_rod_rotation)
        vals.append(self._human_2_rod_linear)
        vals.append(self._human_2_rod_rotation)
        vals.append(self._human_5_rod_linear)
        vals.append(self._human_5_rod_rotation)
        vals.append(self._human_3_rod_linear)
        vals.append(self._human_3_rod_rotation)
        return vals



