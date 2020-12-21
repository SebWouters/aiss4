aiss4: suffix array via induced sorting
---------------------------------------

aiss4 provides an implementation of

Ge Nong, Sen Zhang and Wai Hong Chan,
Linear Suffix Array Construction by Almost Pure Induced-Sorting,
2009 Data Compression Conference, Snowbird, UT,
pages 193-202 (2009) [1]


The name aiss4 is the Burrows-Wheeler transformation (bwt) of sais
(Suffix Array via Induced Sorting):

    $
    ais$
    is$
    s$
    sais$

* src/sais_basic.hpp contains a basic implementation based on the paper
[1] and an online walk-through of that paper [2]
* src/sais.hpp contains an implementation based on Yuta Mori's highly 
optimized sais, version 2.4.1 [3]
* src/bwt.hpp contains an implementation of the Burrows-Wheeler
transformation (encoding and decoding)

The aim of the project is personal, to learn the SA-IS algorithm.
Although timings for chr22.dna and etext99 of the Manzini and
Ferragina corpus [4] are similar to sais 2.4.1, it's probably better
to use sais [3] or libdivsufsort [5].

[1] https://doi.org/10.1109/DCC.2009.42
[2] https://zork.net/~st/jottings/sais.html
[3] https://sites.google.com/site/yuta256/sais
[4] http://people.unipmn.it/manzini/lightweight/corpus/
[5] https://github.com/y-256/libdivsufsort

Don't forget to unzip the files in data!


Bugs, remarks & questions
-------------------------

--> sebastianwouters [at] gmail [dot] com

Copyright
---------

Copyright (c) 2020, Sebastian Wouters

All rights reserved.

aiss4 is licensed under the BSD 3-Clause License. A copy of the License
can be found in the file LICENSE in the root folder of this project.

