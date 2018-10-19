DigitalCurling
====
## OverView
 * Digital Curling is a system which simulates a curling game /w Box2D (box2d.org).

 * This system enables competitive play between two Curling AIs.
 
 * This system is developed in Ito.Lab in University of Electoro-Communication.

 * More details on official site (http://minerva.cs.uec.ac.jp/curling/wiki.cgi).

## Description
### Simulator
* Simulates the motion of stones with 2D physics simulator (Box2D).
* Applies normal random number to initial velocity vector of a shot.
* Class 'b2simulator::Simulator' provides functions for simulation and creating shots. 
### Server
* Controls the processe of Digital Curling game.
* Plays game with normal rule or Mix Doubles rule.
* Communicates with Curling AIs on local.
* Communicates with Curling AIs with Digital Curling Protocol (DCP).
### SampleAI
* A sample for Curling AI.
* Uses simulation, creating shots, constant values from the simulator.

## Digital Curling Protocol (DCP)
* DCP provides command for communication between the server and Curling AIs.
* Discription for each commands here. 
* (http://minerva.cs.uec.ac.jp/curling/wiki.cgi?page=DigitalCurlingProtocol).
~~~
Server                          CurlingAI
   | ---------- ISREADY ----------> |
   | <--------- READYOK ----------- |
   |                                |
   | ---------- PUTSTONE ---------> | + Only for Mix Doubles rule
   | <--------- PUTSTONE ---------- | +
   |                                |
   | ---------- NEWGAME ----------> |
   | ---------- GAMEINFO   -------> |
   | ---------- RANDOMSIZE -------> |
   |                                |
   | ---------- SETSTATE ---------> | +
   | ---------- POSITION ---------> | | Repeats while a game continues
   | ---------- GO ---------------> | |
   | <--------- BESTSHOT ---------- | +
   |                                |
   | ---------- SCORE ------------> | + After an end
   |                                |
   | ---------- GAMEOVER ---------> | + After a game
~~~

## Build