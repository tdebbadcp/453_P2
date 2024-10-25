Trilok Debbad, Siddarth Maddhula

Two ways this LWP differs from a regular library is that the LWP is managed at the user-level by an application, while a real thread management library is managed by the OS itself. 
The simplified nature of an LWP also means that some of the more complicated thread management functions like preemption and thread pooling are absent.

Current issues with the implementation resolve around the exit and wait functions being unifinished. Along with
those potential improvements, the stack setup needs to be ironed out quite a bit more. There's some misalignment there.
Segfaults were common in testing.


