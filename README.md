# RT-NRT_IPC_Xenomai
Inter Process Communication Mechanism, Real / Non-Real Time, OS Xenomai


# XENOMAI INSTALLATION

Note: I used this configuration to install Xenomai:

1) linux-4.14.134
2) xenomai-3.0.9
3) ipipe-core-4.14.134-x86-8.patch

## Download and unzip the package in usr/src

```
open the terminal:
wget https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.14.134.tar.gz
wget https://xenomai.org/downloads/xenomai/stable/xenomai-3.0.9.tar.bz2
wget https://xenomai.org/downloads/ipipe/v4.x/x86/ipipe-core-4.14.134-x86-8.patch
sudo cp linux-4.14.134.tar.gz xenomai-3.0.9.tar.bz2 ipipe-core-4.14.134-x86-8.patch /usr/src/

cd /usr/src
sudo tar xvzf linux-4.14.134.tar.gz
sudo tar xvjf xenomai-3.0.9.tar.bz2
sudo rm -rf linux-4.14.134.tar.gz
sudo rm -rf xenomai-3.0.9.tar.bz2 

```

```
sudo apt-get install libncurses5 libncurses5-dev build-essential kernel-package fakeroot libssl-dev
```

## Apply the Xenomai patch

```
cd /usr/src/linux-4.4.43

/usr/src/xenomai-3.0.9/scripts/prepare-kernel.sh --arch=x86_64 --linux=/usr/src/linux-4.14.134 --ipipe=/usr/src/ipipe-core-4.14.134-x86-8.patch

```
## Configure the kernel

```
sudo cp RT-NRT_IPC_Xenomai/conf/kernel_config .config

```
## RUN

```
export CONCURRENCY_LEVEL=4

sudo make bzImage modules

sudo make modules_install

sudo make install 

```
## Allow non-root users

```
sudo addgroup xenomai --gid 1234
sudo addgroup root xenomai
sudo usermod -a -G xenomai $USER

```
Tip

If the addgroup command fails (ex: GID xenomai is already in use), change it to a different random value, and report it in the next section.

## Configure GRUB and reboot

```
sudo nano /etc/default/grub

```
Insert only this line:
```
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash xenomai.allowed_group=1234"

```
Note

Please note the xenomai group (here 1234) should match what you set above (allow non-root users).

Tip

noapic option might be added if the screen goes black at startup and you can’t boot.

```
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash noapic xenomai.allowed_group=1234"

```

Then:

```
sudo reboot.

You will find another linux kernel into Advanced options for Ubuntu, Linux-4.14.134

```

## Install Xenomai


```
./configure --with-core=cobalt --enable-smp --enable-pshared
make install

```

## Update your bashrc

```
echo '
### Xenomai
export XENOMAI_ROOT_DIR=/usr/xenomai
export XENOMAI_PATH=/usr/xenomai
export PATH=$PATH:$XENOMAI_PATH/bin:$XENOMAI_PATH/sbin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$XENOMAI_PATH/lib/pkgconfig
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$XENOMAI_PATH/lib
' >> ~/.xenomai_rc

echo 'source ~/.xenomai_rc' >> ~/.bashrc
source ~/.bashrc
```
## RUN (every boot of xenomai kernel)

```
run sudo chgrp xenomai /dev/rtp* && sudo chmod g+rw /dev/rtp*
run sudo /usr/xenomai/sbin/autotune
```

## Test your installation

```
xeno latency
```

![latency](https://github.com/ADVRHumanoids/XBotBlock/blob/master/cmakeBlockFactory3.png)

Tip

To get pertinent results, you need to stress your system while running the latency test. The latency has to be stable even if the system is under load.

```
sudo apt install stress
# Using stress
stress -v -c 8 -i 10 -d 8
```

