# aktuellt-alarmclock
This repository is supposed to act as a rasperry pi based alarm clock that can trigger a playback of the latest news episode on a scheduled time



## play video
From mp4:
```bash
mpv --fs ~/videos/filename.mp4
```


Build backend
```bash
cd ~/aktuellt-alarmclock/backend
mkdir -p build
cd build
cmake ..
cmake --build .
```

Verify backend health from other device:
```bash
user@otherComputer ~ % curl -s http://hemma-hallon.local:8080/api/health | jq
{
  "message": "Backend is healthy",
  "data": {
    "status": "ok"
  },
  "success": true
}
```

## Installing systemd services
```bash
sudo cp systemd/aktuellt-play.service /etc/systemd/system/aktuellt-play.service
sudo systemctl daemon-reload
```