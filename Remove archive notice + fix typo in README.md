This is the repository of BIRE_ADV(bad name I know...).



What is BIRE_ADV ?



It's a Brainfuck VM, I originally intented to only

support interpreted execution and then...

I was like << EHHHH, I want to experiment with JIT on macOS ! >>


And I managed to make a pretty dumb JIT that works !



Note that this is a reupload of my project, with a few bug fixes ! :D


Don't judge the poor quality of the code please !

(I'm tired soooo, and it's a personal project)


I was compiling using Clang during my testing.

On a Apple M4 :

Interpreter Mode running mandelbrot.bf = 4.48 seconds !

JIT Mode running mandelbrot.bf = 0.63 seconds !


Pretty good for a non-optimizing/dumb/poor quality JIT ! :D



Update :


After implementing pretty basic DSE(Dead Store Elimination) and a few other optimizations...


I managed to reduce runtime from 0.63 seconds to 0.50 seconds for mandelbrot.bf ! Not very impressive I know...


And by the way... Due to a few changes in the IR(necessary for my optimizations) that completely broke

Interpreter mode, and honestly ? Couldn't care less ! I'll fix it one day if I feel like it !

Why ? The main point of this project was playing around with macOS JIT !


Note : The JIT doesn't work on x86_64 CPUs(pretty logical).

Note 2 : The JIT only works on macOS ! (tested on macOS 26.3)


You're free to do whatever you want with my code !

(Just don't redistribute it without crediting me !)


Copyright (c) 2026 - Yann BOYER
