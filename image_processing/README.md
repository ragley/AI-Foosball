Documentation for the Image Processing portion of the AI-Foosball Spring 2022 Senior Design Project.

If you are reading this, and are not familiar with either *Python* or *UNIX* terminal commands, then I'd **highly** suggest familiarizing yourself with them.

# Technologies In Use and Their Purpose
- OpenCV
- Python3
- Sockets

# Alternatives
- How and in what ways can things be done differently, and what things will the programmer have to add to do them.

# Running and Startup Execution
`sudo cp /bin/ball_tracking.py /bin`

This program is designed to run on startup. If you are developing this application (and obviously don't want the previous code to run on startup run `sudo crontab -e` and remove the line `@reboot python3 /bin/ball_tracking.py`.

Conversly, to add you program for execution at startup run `sudo crontab -e` and add `@reboot python3 /bin/"your_app_here.py"` at the bottom of the file.

For normal execution, `sudo python3 "your_app_here.py"` is sufficient. 
