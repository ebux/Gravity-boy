@echo off
hl3ekezet.exe gravity_boy.c _gravity_boy.c
zcc.exe +homelab2 gravity_boy.c -a
zcc.exe +homelab2 _gravity_boy.c -m
hl3bin2htp.exe a.bin gravity_boy.htp gravity_boy
del a.bin
rem del _gravity_boy.c



