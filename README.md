# DigitalCurling

## OverView
 * Digital Curling is a system which simulates a curling game /w Box2D (box2d.org).
 * This system enables competitive play between two Curling AIs.
 * More details on official site (http://minerva.cs.uec.ac.jp/curling/wiki.cgi).

## Description
### Simulator
* Simulates the motion of stones with 2D physics simulator (Box2D).
* Applies normal random number to initial velocity vector of a shot.
* Class *Simulator* provides functions for simulation and creating shots. 


### Server
* Controls the processe of Digital Curling game.
* Plays game with normal rule or Mix Doubles rule.
* Communicates with Curling AIs on local.
* Communicates with Curling AIs with Digital Curling Protocol (DCP).
* Outputs log file as DCL (.dcl), which can be replayed with DigitalCurling Client.

### SampleAI
* A sample for Curling AI.
* Uses simulation, creating shots, constant values from the simulator.

## Build
* Open *DigitalCurling.sln*.
* Build project *Simulator* at first (*Solution Explorer* -> *Simulator* -> *'build project'*).
* Build project *Server* or *SampleAI*.
   * You need to add *Simmulator.lib* to additional dependancies of each project.

## Run
* Run `Server.exe` with console.
* `help` or `h` to show all commands.
* `run` : run single match 'match_default' from  *config.json*.

## Digital Curling Protocol (DCP)
### Overview
* DCP provides command for communication between the server and Curling AIs.
* Each message contains a command and arguments such as `COMMAND arg1 arg2...`.

### -> Details of each commands on [DCP.md](https://github.com/digitalcurling/DigitalCurling/blob/master/DCP.md)

## Mix Doubles Curling