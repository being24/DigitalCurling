## Digital Curling Protocol (DCP) ver.1.1
### Overview
* DCP provides command for communication between the server and Curling AIs.
* Each message contains a command and arguments such as `COMMAND arg1 arg2...`.

~~~
Server                          CurlingAI
   | ---------- ISREADY ----------> |
   | <--------- READYOK ----------- |
   |                                |
   | ---------- NEWGAME ----------> |
   | ---------- GAMEINFO ---------> |
   | ---------- PLAYERINFO -------> |
   |                                |
   | ---------- SETORDER ---------> | + Omitted if num_players == 1
   | <--------- SETORDER ---------- | +
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
* For confirm an AI is ready.
* AI should return `READYOK` after reciving `ISREADY`.

#### `NEWGAME name1 name2`
* Notifies a new game will start.
* `name 1`, `name2` : Name of each player

#### `GAMEINFO rule random num_players`
* Notifies information about this match.
* `rule` : type of rule (0: normal, 1:Mix Double)
* `random` : type of random generator (0: Rectangular, 1: Polar)
* `num_players` : number of players in 1 team

#### `PLAYERINFO rand1_0 rand2_0 shotmax_0 ... rand1_3 rand2_3 shotmax_3`
* Notifies parameters of each player.
* `rand1_n` : size of random number 1 (x in Rectangular, v in Polar) for n th player
* `rand2_n` : size of random number 2 (y in Rectangular, theta in Polar) for n th playr
* `shotmax_n` : max size of shot vector
*  The number 'n' is up to 1 (Mix Doubles) or 3 (Normal rule).

#### `SETORDER`
#### `SETORDER m0 ... m3`
* Set order for each shot.
* AI should return order of players after recieving this command.
* This command is omitted if `num_players` = 1 in `GAMEINFO` command.
* Example of order below:

`SETORDER 0 2 1 3` (Normal rule):  

| shot number** | random_1 | random_2 | shot_max |
| :---: | :---: | :---: | :---: |
| 0 | rand1_0 | rand2_0 | shotmax_0 |
| 1 | rand1_2 | rand2_2 | shotmax_2 |
| 2 | rand1_1 | rand2_1 | shotmax_1 |
| 3 | rand1_3 | rand2_3 | shotmax_3 |
| 4 | rand1_0 | rand2_0 | shotmax_0 |
| 5 | rand1_2 | rand2_2 | shotmax_2 |
| 6 | rand1_1 | rand2_1 | shotmax_1 |
| 7 | rand1_3 | rand2_3 | shotmax_3 |

`SETORDER 0 1` (Mix Doubles):  

| shot number** | random_1 | random_2 | shot_max |
| :---: | :---: | :---: | :---: |
| 0 | - | - | - |
| 1 | - | - | - |
| 2 | - | - | - |
| 3 | rand1_0 | rand2_0 | shotmax_0 |
| 4 | rand1_1 | rand2_1 | shotmax_1 |
| 5 | rand1_1 | rand2_1 | shotmax_1 |
| 6 | rand1_1 | rand2_1 | shotmax_1 |
| 7 | rand1_0 | rand2_0 | shotmax_0 |

*Each parameters are from `PLAYERINFO` command  
**'shot number' is number for each player

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
* If `y` is bigger than `shotmax_n` from `PLAYERINFO` command,  
the shot will be `0 0 0` as illegal shot.

#### `CONCEDE`
* To give up a match.
* The match will immidiately finish after recieving `CONCEDE`. 

#### `SCORE score0 ... score9`
* Notifies the scores.
* `scoren` : score of nth end (>0: 1st player scored, <0: 2nd player scored)

#### `GAMEOVER win`
* Notifies the  match finished.
* `win` : `WIN` or `LOSE` or `DRAW`

---

#### `RANDOMSIZE size1 size2` *abolition
* Notifies size of random numbers.
* `size1`, `size2` : Size of random numbers (x,y) or (v, theta)
