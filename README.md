# AI-Foosball

# Running Python Scripts at Startup

Follow the instructions here: <https://unix.stackexchange.com/questions/634410/start-python-script-at-startup> it's fairly self-explainatory.

For our startup scripts we copied the python codes we wanted to run at start into the `/bin` folder. This allows the script to be run with superuser permissions. For reference, the command to copy to the `/bin` directory is:
`sudo cp ./"script name" /bin/"script name"`

This would mean the .service file would like like:

```
[Unit]
Description=My Script

[Service]
ExecStart=/usr/bin/python3 /bin/"your script here".py

[Install]
WantedBy=multi-user.target
```

On thing of note however, this will not work for any _graphical_ application (i.e. a gui). The script will also crash if you try to open a window (such as to display the camera output). It handles print statements fine, athough you won't see them unless you run `sudo systemctl status "your service here".service`.

The alternative is to use a `crontab` to set things at startup, but you'll have to figure that out yourself.
