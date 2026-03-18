Version 0.0.2
=============

Changes:
	- Updated integers to 64 bit by default
	- Setup profiling using Google Perf Tools. Attempted to optimize
		- Docs: https://gperftools.github.io/gperftools/cpuprofile.html
		- brew install gperftools
		- go install github.com/google/pprof@latest

Fixes:
	- Fixed chunk cache hashmap filling up completely resulting in extremely bad 
	chunk lookup time.

Version 0.0.1
=============

Features:
	- Basic world gen
	- Infinite worlds
	- Minecraft style ambient occlusion