<p align=center>
    <img src=assets/Rootkitty.png width=50%></img>
</p>
<h1 align=center>Rootkitty</h1>
<p align=center>simple and detectable rootkit using LD_PRELOAD</p>

## **DISCLAIMER**
> [!IMPORTANT]
> This open-source tool is provided for educational and research purposes only. 
> I, the creator, expressly prohibit the use of this tool by governments, threat actors, or individuals with malicious intent. 
> I, the creator, disclaim any responsibility for any damages caused to systems by the use of this tool or for any malicious activities conducted using this tool in the wild. 
> Users are solely responsible for their use of this tool and are expected to adhere to ethical and legal standards at all times. 

## Features:
- Hides files                                                   
- Intercepts `SSL_write`
- Questionable Anti-Debugging   (WIP)
- PAM Backdoor for easy privesc (WIP)
- Reverse Shell over SystemD service (WIP)

## Issues:
- Doesnt run on WSL

## Compiling:
Seriously? If you can't even use a Makefile you shouldn't even be allowed to use this. :)
