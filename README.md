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
