# LogiProbe
## ARM Cortex-M4 Based Logic Analyzer

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Architecture](#software-architecture)
- [Building and Using](#building-and-using)
- [Performance](#performance)
- [Future Development](#future-development)
- [Contributors](#contributors)
- [License](#license)

## Overview

LogiProbe is a logic analyzer implementation based on the STM32F429 Discovery Board. It features DMA-based sampling in both state and timing modes, with configurable sampling options and protocol analysis capabilities.

## Features

### Dual Mode Operation
* **State Mode**
  * Sampling on external clock edges
  * Edge-configurable sampling (Rising/Falling/Both)
  * Pattern-based triggering support

* **Timing Mode**
  * Sampling on internal clock edges
  * Configurable sampling frequencies
  * Button-triggered acquisition

### High Performance
* Up to 3 MHz sampling in state mode
* Up to 1 MHz sampling in timing mode
* 8MB SDRAM buffer using FMC
* DMA-based data acquisition

### Protocol Analysis
* I2C protocol decoder with support for:
  * START/STOP detection
  * Address & data parsing
  * ACK/NACK handling
* Extensible framework for additional protocols

### Data Storage & Visualization
* SD Card storage using FAT16
* Python-based waveform visualization
* Interactive plotting interface

## Hardware Requirements

### Required Components
* STM32F429 Discovery Board
* SD Card module (SPI interface)
* USB to TTL converter
* General Purpose board for connections

### Pin Configuration

```
Channel Inputs:
- PC8-PC15: Channel 0-7

State Mode Clock Input:
- PA9: TIM1 Input Capture
- PC7: TIM8 Input Capture

SD Card Interface:
- PA5: SCK
- PA6: MISO
- PA7: MOSI
- PB12: CS

UART Interface:
- PD5: TX
- PD6: RX
```

## Software Architecture

### Core Components

#### 1. DMA Controller
* Uses DMA2 for GPIO sampling
* Supports double buffering for trigger detection
* Synchronized transfers using timer events

#### 2. Flexible Memory Controller (FMC)
* Interfaces with onboard 8MB SDRAM
* Memory-mapped for direct access
* Configurable timing parameters

#### 3. Protocol Analyzers
* I2C decoder implementation
* Extensible framework
* Real-time processing

#### 4. Command Processor
* UART-based interface
* Command line parameter parsing
* Configurable acquisition settings

## Building and Using

### Development Environment

**Required Software:**
* STM32CubeIDE
* Python with packages:
  * numpy
  * matplotlib
  * ipywidgets

### Building Steps

1. Clone the repository:
2. Open in STM32CubeIDE
3. Build and flash to STM32F429 Discovery Board

### Command Reference

#### 1. Timing Mode (TMODE)
```bash
tmode -f <freq> -i <interpreter> -s <size> -m <mode>
```
* `-f`: Sampling frequency [100,200,400,800,1000] kHz
* `-i`: Protocol interpreter [i2c]
* `-s`: Buffer size [s,m,l]
* `-m`: Acquisition mode [button]

#### 2. State Mode (SMODE)
```bash
smode -e <edge> -m <mode> -s <size> -p <pin> -t <pattern> -d <delay>
```
* `-e`: Sampling edge [r,f,b]
* `-m`: Mode [button,trigger]
* `-s`: Buffer size [s,m,l]
* `-p`: Trigger pin [0-7]
* `-t`: Trigger pattern [hex]
* `-d`: Trigger timeout [ms]

#### 3. Analyze
```bash
analyse -m <mode> -s <size>
```
* `-m`: Analysis mode [i2c]
* `-s`: Data size [s,m,l]

#### 4. Save
```bash
save -s <size>
```
* `-s`: Data size to save [s,m,l]

### Example Usage

1. I2C Communication Capture:
```bash
tmode -i i2c -f 200
```

2. State Mode with Trigger:
```bash
smode -m trigger -p 0 -t 0xAA -e r
```

3. Analyze Captured Data:
```bash
analyse -m i2c
```

## Performance

* **State Mode:** Tested up to 3 MHz
* **Timing Mode:** Tested up to 1 MHz
* **SDRAM:** Operating at 80 MHz
* **Buffer Capacity:** 8 seconds at 1 MHz sampling

## Future Development

- [ ] Add hardware interface controls
- [ ] Implement display module for direct visualization
- [ ] Add support for SPI and UART protocol analysis
- [ ] Enhance trigger capabilities

## Contributors

* **Krish Shah**
* **Pranjal Gupta**

## License

This project is released under the MIT License. See the [LICENSE](LICENSE) file for details.

---

*Developed as part of ECEN 5613 Embedded System Design at University of Colorado Boulder*
