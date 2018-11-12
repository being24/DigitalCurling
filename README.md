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
* Class `Simulator` provides functions for simulation and creating shots. 


### Server
* Controls the processe of Digital Curling game.
* Plays game with normal rule or Mix Doubles rule.
* Communicates with Curling AIs on local with Digital Curling Protocol (__DCP__).
* Outputs log file as DCL (.dcl), which can be replayed with DigitalCurling Client.

### SampleAI
* A sample for Curling AI.
* Uses simulation, creating shots, constant values from the simulator.

## Build
* Open `DigitalCurling.sln` and build the solution.
* To build _Server_ and _SampleAI_,  
you should add `Simulator.lib` (after build _Simulator_) to additional dependencies.

---

## Digital Curling Protocol (DCP)
### Overview
* DCP provides command for communication between the server and Curling AIs.
* Each message contains a command and arguments such as `COMMAND arg1 arg2...`.

~~~
Server                          CurlingAI
   | ---------- ISREADY ----------> |
   | <--------- READYOK ----------- |
   |                                |
   | ---------- NEWGAME ----------> |
   | ---------- GAMEINFO   -------> |
   | ---------- RANDOMSIZE -------> |
   |                                |
   | ---------- PUTSTONE ---------> | + Only for Mix Doubles rule
   | <--------- PUTSTONE ---------- | | Repeats while a match continues
   |                                | |
   | ---------- SETSTATE ---------> | | +
   | ---------- POSITION ---------> | | | Repeats while an end continues
   | ---------- GO ---------------> | | |
   | <--------- BESTSHOT ---------- | + + or `CONCEDE` to give up
   |                                |
   | ---------- SCORE ------------> | + After an end
   |                                |
   | ---------- GAMEOVER ---------> | + After a game
~~~

### Commands
#### `ISREADY`
#### `READYOK`
* To confirm an AI is ready.
* AI should return `READYOK` after reciving `ISREADY`.
#### `NEWGAME name1 name2`
* Notifies a new game will start.
* `name 1`, `name2` : Name of each player
#### `GAMEINFO rule random`
* Notifies information about this match.
* `rule` : type of rule (0: normal, 1:Mix Double)
* `random` : type of random generator (0: Rectangular, 1: Polar)
#### `RANDOMSIZE size1 size2`
* Notifies size of random numbers.
* `size1`, `size2` : Size of random numbers (x,y) or (v, theta)
#### `PUTSTONE`
#### `PUTSTONE type`
* For putting stone (only Mix Doubles).
* `type` : type of positions
#### `SETSTATE shot c_end l_end move`
* Notifies information about current state.
* `shot` : Number of shot
* `c_end` : Number of current end
* `l_end` : Number of last end
* `move` : Which player has next stone (0: first, 1: second in first end)
#### `POSITION x0 y0 ... x15 y15`
* Notifies positions of 15 stones.
* `xn`, `yn` : x, y coordinate of nth stone
#### `GO timelimit1 timelimit2`
* Notifies the thinking time is start.
* `timelimmit` : timelimit for 1st and 2nd player in 1st end (miliseconds)
* AI should return `BESTSHOT`/`CONCEDE` command in timelimit [ms].
#### `BESTSHOT x y angle`
* Returns the shot vector to throw.
* `x` : x coordinate of shot vector
* `y` : y coordinate of shot vector
* `angle` : angle of curl (0: right, 1:left)
#### `CONCEDE`
* To give up a match.
* The match will immidiately finish after recieving `CONCEDE`. 
#### `SCORE score0 ... score 9`
* Notifies the scores.
* `scoren` : score of nth end (>0: 1st player scored, <0: 2nd player scored)
#### `GAMEOVER win`
* Notifies the  match finished.
* `win` : `WIN` or `LOSE` or `DRAW`

---

## Mix Doubles Curling