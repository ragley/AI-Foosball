# test reading json

import json
import numpy as np


with open('calibration.json') as json_file:
	data = json.load(json_file)
	dim_width = data['dim_width']
	dim_height = data['dim_height']
	DIM = (int(dim_width), int(dim_height))
	K=np.array(data['k'])
	D=np.array(data['d'])
	# print(type(K))
	# print(type(D))
	# print(type(DIM))

DIM=(640, 360)
K=np.array([[529.7982121993252, 0.0, 312.7158779979683], [0.0, 527.7751535552674, 175.59551889975003], [0.0, 0.0, 1.0]])
D=np.array([[-0.10259242250486603], [-0.4053339824788657], [2.145677156701134], [-3.3630763147728104]])

print(type(K))
print(type(D))
print(type(DIM))
