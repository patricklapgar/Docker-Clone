# Docker Clone

## Description

A lightweight OS-level containerization tool developed in C with some help from a small Shell script and Dockerfile.

C was employed primarily to serve as gentle practice for me to sharpen my C skills. 

## What it does:
 - Process isolation
 - Filesystem isolation
 - Handles exit codes

## What needs to be worked on:
 - Networking stuff (i.e. container linking, multi-host networking, etc)
 - GPU Support (giving access to GPU devices like NVIDIA graphics cards)
 
 ## Challenges
 
The most challenging part of this project so far has to be filesystem isolation.

Although the end result isn't too complicated, I had to ensure a chroot environment was properly set up or else there would be extremely negative effects.
