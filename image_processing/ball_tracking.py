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

	# construct the argument parse and parse the arguments
	ap = argparse.ArgumentParser()
	ap.add_argument("-b", "--buffer", type=int, default=64, help="max buffer size")
	args = vars(ap.parse_args())

	# define the lower and upper boundaries of the "green"
	# ball in the HSV color space, then initialize the
	# list of tracked points
	# greenLower = (49, 106, 26)
	# greenUpper = (64, 255, 255)
	greenLower = (170, 80, 80)
	greenUpper = (180, 200, 200)
	pts = deque(maxlen=args["buffer"])

	# if a video path was not supplied, grab the reference
	# to the webcam
	vs = VideoStream(0)
	vs.start()

	# otherwise, grab a reference to the video file
	# allow the camera or video file to "warm up"
	time.sleep(2.0)

	# begin processing the image. 
	# reset/define important variables
	previous_x = 0
	previous_y = 0
	t2=0
	while True:
		# performance testing
		time1 = cv2.getTickCount()

		# grab the current frame
		frame = vs.read()
		
		# handle the frame from VideoCapture or VideoStream
		# frame = frame[1] if args.get("video", False) else frame
		
		# resize the frame, blur it, and convert it to the HSV
		# color space
		frame = imutils.resize(frame, width=600)
		blurred = cv2.GaussianBlur(frame, (11, 11), 0)
		hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
		
		# construct a mask for the color "green", then perform
		# a series of dilations and erosions to remove any small
		# blobs left in the mask
		mask = cv2.inRange(hsv, greenLower, greenUpper)
		# mask = cv2.erode(mask, None, iterations=2)
		# mask = cv2.dilate(mask, None, iterations=2)
		
		# find contours in the mask and initialize the current
		# (x, y) center of the ball
		cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
			cv2.CHAIN_APPROX_SIMPLE)
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
				if radius > 2:
					# draw the circle and centroid on the frame,
					# then update the list of tracked points
					cv2.circle(frame, (int(x), int(y)), int(radius),
						(0, 255, 255), 2)
					cv2.circle(frame, (int(x), int(y)), 5, (0, 0, 255), -1)
		except:
			# to avoid crashing on divideByZero error
			continue
		
		# update the points queue
		pts.appendleft(center)

		# performance check
		time2 = cv2.getTickCount()

		# show the frame to our screen
		cv2.imshow("Frame", frame)
		key = cv2.waitKey(1) & 0xFF
		# if the 'q' key is pressed, stop the loop
		if key == ord("q"):
			break
		
		print(f" Processing Time (ms): {((time2-time1)/cv2.getTickFrequency())*1000}") 
	
	# stop camera
	vs.stop()

ball_tracking()
cv2.destroyAllWindows()
