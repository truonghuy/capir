### CAPIR (Collaborative Action Planning with Intention Recognition)

This project includes codes to create simple collaborative games. This code repository includes:
* CAPIRSolver: C++ codes used to compute behavior policies.
* Unity codes: The game codes 
* Compiled game executables: Mac OS X and Win32 
* CAPIR User Manual 

#### Demos

[![Video to the demo 1](http://img.youtube.com/vi/nTYQ2Oo1udA/0.jpg)](http://www.youtube.com/watch?v=nTYQ2Oo1udA)

[![Video to the demo 2](http://img.youtube.com/vi/Kn3CWDXw2Hw/0.jpg)](http://www.youtube.com/watch?v=Kn3CWDXw2Hw)

#### Technical Description

We address the Lead-Assistant Collaboration problem that characterizes an 
autonomous assistant aiding an agent in achieving multiple goals in a known 
environment; the main source of uncertainty is the agent's drifting intention 
and behavior model in achieving the goals. We propose the CAPIR framework 
(Collaborative Action Planner using Intention Recognition) to solve this problem 
by tracking the assisted agent's intention using inverse planning (a concept 
inherited from cognitive science), and constructing actions based on the 
assistant's belief on the agent's intended goal and its knowledge of solving 
each goal (modeled as a Markov decision process). For complicated goals that yield 
large planning space, simulation-based algorithms are devised to approximately 
solve the goals online. Evaluation with human players in an experiment game shows 
that the assistant's resultant course of actions is near human-level.

#### Related Publications

* Truong-Huy D. Nguyen, Tomi Silander, Lee Wee Sun and Leong Tze Yun (2014).  
**Bootstrapping Simulation-based Algorithms with a Suboptimal Policy**.  
ICAPS 2014. [PDF](https://dl.dropboxusercontent.com/u/76930/Personal/publications/Nguyen_ICAPS2014.pdf)
* Truong-Huy D. Nguyen, Lee Wee Sun and Leong Tze Yun (2012).  
**Bootstrapping Monte Carlo Tree Search with an Imperfect Heuristic**.  
ECML/PKDD 2012. [PDF](https://dl.dropboxusercontent.com/u/76930/Personal/publications/Nguyen_ECML2012.pdf)
* Truong-Huy D. Nguyen, David Hsu, Lee Wee Sun, Leong Tze Yun, Leslie Pack Kaelbling, 
Tomas Lozano-Perez, Andrew Haydn Grant (2011).  
**CAPIR: Collaborative Action Planning with Intention Recognition**.  
AIIDE 2011.
[PDF](https://dl.dropboxusercontent.com/u/76930/Personal/publications/Nguyen_AIIDE2011.pdf)
