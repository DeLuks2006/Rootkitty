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
- PAM Backdoor for *local privilege escalation* (WIP)
- Reverse-Shell over SystemD service

## Description
Rootkitty is my first ever Linux User-Mode Rootkit written in C. 
To use it you either have to add the path to the library to `/etc/ld.so.preload` 
or export the `LD_PRELOAD` environment variable and restart various daemons. 

Additionally rootkitty will add a systemd service that will cause a Connect-Back shell to run at system startup.

Finally Rootkitty will check if a debugger is present before executing some functions. 

## Issues:
- Rootkit does not work when compiled on ubuntu 22 or any distro with glibc version 2.35.
- PAM Backdoor does not work.

## Contributing

Before creating a pull request, please make sure to test the code locally to ensure that it functions as expected. This will help streamline the review process and ensure that the changes are in line with the project's goals.

When submitting code changes, please keep the code simple and follow the existing coding style and conventions used in the project. This will make it easier for maintainers to review and merge your contributions.

If you have any questions or need assistance with contributing, feel free to reach out to me via a Issue.

## Compiling:
Seriously, if you can't even use a Makefile you shouldn't even be allowed to use this. :)
