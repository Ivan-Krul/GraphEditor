GraphEditor
===============
It's a program, which can do graphs. It's an editor, but not visualizer.
It can manage any type of graph, but generally it can do undirected weighted graphs.
For saving/loading your graphs you can use Python scripts to make the program more flexible.
Also you can use CLI tools to make advanced interface for your programs

Help
===============
```
Commands:
	newp - create a node
	newe - create an edge (from origin to...)
	seto - set origin node (to...)
	remp - remove a node and removing all references to the node (is...)
	reme - remove an edge between origin and a node (from origin to...)
	list - listing all nodes and costs which are adjacent to the origin node
	dict - shows a dictionary of names and it's translation to their indexes
	save - saves graph in .grf using custom Python scripts
	load - loads graph from .grf using custom Python scripts
	clir - clears the screen
	rset - resets graph to empty graph
	exit - exit

Also exists custom arguments:
	[--version | -v]  - get a version
	[-d]              - enter to debug mode (cache wouldn't be erased)
	[-h | --help]     - shows help for navigating the program
	[--argument | -a] - enter to argument mode (you can write all commands in
                  arguments separated by space (for names as indexes you have to
				  type 'n' and space before actual name))
```

Installing
===============
In realese tab exists 6 binaries for x86_32 and x86_64
Also it's divided into 3 categories:
 -  Linux (in general with GCC)
 -  Windows MSVC
 -  Windows MinGW

Also you have to install
 -  Python3
 -  necessary DLLs for working properly (Windows only)

You just double click and that it (in Linux, launch using terminals💀)


Compiling
===============
If you're wanting to compile using MinGW or GCC, then you can just launch `build.sh`.
If you're wanting to compile using MSVC, then here's a .sln project

It requires:
 -  c++17

That's it
===============
