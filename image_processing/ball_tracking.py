# import the necessary packages
# python ball_tracking.py
# When the app is running press 'q' to cleanly stop the app.
from collections import deque
from imutils.video import VideoStream
import numpy as np
import argparse
import cv2
import imutils # pip install --upgrade imutils
import time

def calculateVelocity(x, y, previous_x, previous_y, t1, t2):
	Vx = (x - previous_x)/(t1-t2)
	Vy = (y - previous_y)/(t1-t2)
	return (Vx, Vy)

def ball_tracking():
	# camera fisheye calibration arrays
	DIM=(640, 360)
	K=np.array([[529.0990431685142, 0.0, 308.01283344567565], [0.0, 527.5594266286947, 168.13007918363098], [0.0, 0.0, 1.0]])
	D=np.array([[-0.11376404127315753], [-0.46295061178792457], [2.167920495942993], [-2.8734014030396717]])

	# construct the argument parse and parse the arguments
	ap = argparse.ArgumentParser()
	ap.add_argument("-b", "--buffer", type=int, default=64, help="max buffer size")
	args = vars(ap.parse_args())

	# define the lower and upper boundaries of the "green"
	# ball in the HSV color space, then initialize the list of tracked points
	# greenLower = (49, 106, 26)
	# greenUpper = (64, 255, 255)
	ballLower0 = np.array([172, 87, 111], np.uint8)
	ballUpper0 = np.array([180, 255, 255], np.uint8)
	
	ballLower1 = np.array([0, 87, 111], np.uint8)
	ballUpper1 = np.array([5, 255, 255], np.uint8)
	
	pts = deque(maxlen=args["buffer"])

	# if a video path was not supplied, grab the reference
	# to the webcam
	vs = VideoStream(0)
	vs.start()

	# otherwise, grab a reference to the video file
	# allow the camera or video file to "warm up"
	time.sleep(2.0)

	# begin processing the image. reset/define 
	# important variables for velocity calculations
	previous_x = 0
	previous_y = 0
	t2=0

	# begin image detection
	while True:
		# performance testing
		time1 = cv2.getTickCount()

		# grab the current frame
		frame = vs.read()
		
		# remove fisheye, new (undistored) frame is used for the remainder of the app
		map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv2.CV_16SC2)
		undistorted_frame = cv2.remap(frame, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
		
		# resize the frame, blur it, and convert it to the HSV
		# color space
		# frame = imutils.resize(frame, width=600)
		blurred = cv2.GaussianBlur(undistorted_frame, (11, 11), 0)
		hsv = cv2.cvtColor(undistorted_frame, cv2.COLOR_BGR2HSV)
		
		# construct a mask for the color "green", then perform
		# a series of dilations and erosions to remove any small
		# blobs left in the mask
		mask0 = cv2.inRange(hsv, ballLower0, ballUpper0)
		mask1 = cv2.inRange(hsv, ballLower1, ballUpper1)
		mask = mask0 | mask1
		mask = cv2.erode(mask, None, iterations=2)
		mask = cv2.dilate(mask, None, iterations=2)
		
		# find contours in the mask and initialize the current
		# (x, y) center of the ball
		cnts = cv2.findContours(mask.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
		cnts = imutils.grab_contours(cnts)
		center = None		

		try:
			# only proceed if at least one contour was found
			# meaning the ball was found...
			if len(cnts) > 0:
				# find the largest contour in the mask, then use
				# it to compute the minimum enclosing circle and
				# centroid
				c = max(cnts, key=cv2.contourArea)
				((x, y), radius) = cv2.minEnclosingCircle(c)
				t1 = cv2.getTickCount()/cv2.getTickFrequency()
				M = cv2.moments(c)
				center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

				# calculate velocity
				# TODO: A normalization function will be needed for ACTUALL Vx/Vy
				if not t2 == 0:
					(Vx, Vy) = calculateVelocity(x, y, previous_x, previous_y, t1, t2)

				# print x, y coord to the terminal
				print(f"Ball Location (x, y): {x}, {y}")
				if not t2 == 0:
					print(f" Ball Speed (Vx, Vy): {Vx}, {Vy}")
				previous_x = x
				previous_y = y
				t2 = t1

				# only proceed if the radius meets a minimum size
				if radius > 1:
					# draw the circle and centroid on the frame,
					# cv2.circle(undistorted_frame, (int(x), int(x)), int(radius),(0, 255, 255), 3)
					cv2.circle(undistorted_frame, (int(x), int(y)), 3, (255, 0, 255), -1)
		except:
			# to avoid crashing on divideByZero error
			continue
		
		# update the points queue
		pts.appendleft(center)

		# performance check
		time2 = cv2.getTickCount()

		# show the frame to our screen
		cv2.imshow("Frame", undistorted_frame)
		key = cv2.waitKey(1) & 0xFF
		# if the 'q' key is pressed, stop the loop
		if key == ord("q"):
			break
		
		print(f" Processing Time (ms): {((time2-time1)/cv2.getTickFrequency())*1000}") 
	
	# stop camera
	vs.stop()

ball_tracking()
cv2.destroyAllWindows()
