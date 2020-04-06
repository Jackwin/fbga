## Introduction
This driver is for fbga project, and used for GPIO hardware access

## Hierarchy
1. drv => This directory includes all function c code for the driver
2. inc => This directory includes all include header file for the driver
3. test-app => This directory includes a application test demo for the driver
4. Makefile => Makefile for compiles the driver

## Usage
 1. "make" in this current directory in PS linux:
	1. "make" is for compiling the driver
	2. "make clean" is for cleaning the compiling result
 2. "gpio_drv.ko" will be found in current directory;
 3. "insmod gpio_drv.ko" will load the driver to kernel
 
 4. The driver provide access functions include:
	1. write
	2. read
	3. lseek
