# <font face="微軟正黑體">ATK_PowerManager(電源管理系統)</font>

<font face="微軟正黑體">本系統由兩個部分所組成(Arduino、Raspberry)，由Raspberry進行偵測網路狀況或遠端電腦開機需求，透過Serial與Arduino進行通訊，並執行電腦開機或網路基地台重啟的動作。</font>

## <font face="微軟正黑體">第二版本</font>

<font face="微軟正黑體">此版本移除伺服馬達控制視訊攝影機角度，</font>

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20180407_005902.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20180407_005931.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20180407_005910.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20180407_005919.jpg" width="100%" >

## <font face="微軟正黑體">第一版本</font>

<font face="微軟正黑體">此版本包含伺服馬達控制視訊攝影機角度，</font>

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20150929_647927.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20150929_484401.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20150929_586462.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20150929_664846.jpg" width="100%" >

<img src="https://github.com/AtkSeeGow/ATK_PowerManager/raw/dev/Image/20150929_619366.jpg" width="100%" >

<font face="微軟正黑體">YouTube：https://www.youtube.com/watch?v=cyWtMj374c0</font>

## <font face="微軟正黑體">系統設定</font>

```
$ sudo raspi-config 
$ sudo reboot 
$ sudo apt-get update 
$ sudo apt-get upgrade 
$ sudo apt-get install vim
$ sudo apt-get install samba
$ sudo apt-get install python-serial
$ sudo apt-get install git
$ sudo apt-get install fping 

$ cd /etc/samba/
$ sudo nano smb.conf
    [Home_Pi]
    comment = Home_Pi
    path = /home/pi/
    browseable = yes
    writeable = yes
    write list = @users
    create mode = 0664
    directory mode = 0775
$ sudo chgrp users ~
$ sudo chmod 2770 ~
$ sudo pdbedit -a -u pi
$ sudo /etc/init.d/samba restart

$ cd ~
$ mkdir Log
$ mkdir Development\ Project
$ cd Development\ Project/
$ git clone https://github.com/AtkSeeGow/ATK_PowerManager.git
$ cd ATK_PowerManager/
$ git checkout dev
$ vim Source/PythonPowerManagerMainProject/Domain/Config.py

$ sudo crontab -e
    @reboot python /home/pi/Development\ Project/ATK_PowerManager/Source/PythonPowerManagerMainProject/NetworkMonitoring.py &
```
