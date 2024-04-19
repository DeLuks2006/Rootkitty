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
- Questionable Anti-Debugging   (WIP)
- PAM Backdoor for easy privesc (WIP)
- Reverse Shell over SystemD service (WIP)

## Issues:
- Doesnt run on WSL
- Not really a issue but "ld.so.preload write" and "execve hook" isnt tested

## Compiling:
Seriously? If you can't even use a Makefile you shouldn't even be allowed to use this. :)
