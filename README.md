# AI-Foosball

# Running Python Scripts at Startup
Follow the instructions here: <https://unix.stackexchange.com/questions/634410/start-python-script-at-startup> it's fairly self-explainatory. 

On thing of note however, this will not work for any *graphical* application (i.e. a gui). The script will also crash if you try to open a window (such as to display the camera output). It handles print statements fine, athough you won't see them unless you run `sudo systemctl status "your service here".service`.

The alternative is to use a `crontab` to set things at startup, but you'll have to figure that out yourself.