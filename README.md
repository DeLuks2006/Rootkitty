<p align=center>
    <img src=assets/Rootkitty.png width=50%></img>
</p>
<h1 align=center>Rootkitty</h1>
<p align=center>Simple and Detectable User Mode Rootkit using LD_PRELOAD</p>

## **DISCLAIMER**
> [!IMPORTANT]
> This open-source tool is provided for educational and research purposes only. 
> The creator, expressly prohibits the use of this tool by governments, threat actors, or individuals with malicious intent. 
> The creator, disclaims any responsibility for any damages caused to systems by the use of this tool or for any malicious activities conducted using this tool in the wild. 
> Users are solely responsible for their use of this tool and are expected to adhere to ethical and legal standards at all times. 
> 
> By using this Software you explicitly agree to these terms and conditions.

## Features:
- Hides files                                                   
- Intercepts `SSL_write`
- Questionable Anti-Debugging
- PAM Backdoor for *local privilege escalation (WIP)
- Reverse-Shell over SystemD service (WIP)

## Description
Rootkitty is my first ever Linux User-Mode Rootkit written in C. 
To use it you either have to add the path to the library to `/etc/ld.so.preload` 
or export the `LD_PRELOAD` variable and let the rootkit plant itself automatically.

Finally in every hooked function it will check if a debugger is present. And additionally
it will attempt to evade `ldd` and `unhide` by renaming the `ld.so.preload` file to something else.

### *For Future Update:*
After that it will try to plant the *PAM Backdoor* by prepending itself to `/etc/pam.d/sudo` 
(hopefully) causing sudo to accept the added Backdoor-Password.

Rootkitty will try to determine, if the machine its on runs on systemd, if that is the case 
it will add a systemd service that will cause a Connect-Back shell to run at system startup.

## Issues:
- Doesnt run on WSL
- Makefile needs ELF modification script (remove unnecessary sections and etc.)
- `ld.so.preload` write, "execve hook", Anti-Debugging and PAM-Backdoor is not tested

FYI: I intentionally didnt encrypt/hash the backdoor password.

## Compiling:
Seriously? If you can't even use a Makefile you shouldn't even be allowed to use this. :)
