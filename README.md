GraphEditor
===============
It's a program, which can do graphs. It's an editor, but not visualizer.
It can manage any type of graph, but generally it can do undirected weighted graphs.
For saving/loading your graphs you can use Python scripts to make the program more flexible.
Also you can use CLI tools to make advanced interface for your programs.
It can also support aliases or functions, where you can speed up your progress.
You can also assisting with variables to produce faster results

Help
===============
```
Commands:
        newp - create a node
        newe - create an edge (from origin to...)
        seto - set origin node (to...)
        remp - remove a node and removing all references to the node (is...)
        reme - remove an edge between origin and a node (from origin to...)
        renm - rename the current node
        list - list all nodes and costs which are adjacent to the origin node
        dict - shows a dictionary of names and it's translation to their indexes
        save - saves graph in .grf using custom Python scripts
        load - loads graph from .grf using custom Python scripts
        clir - clears the screen
        help - shows help for navigating the program
        rset - resets graph to empty graph
        lsta - list whole graph with all edges of nodes
        tmpi - saves as cache for performance
        tmpo - loads cache from command 'tmpo' or from 'load' with debug flag
        newf - creates a function was for argument list and saves in .func
        lstf - list loadedd functions
        call - call a function, which execute all from argument list
        reff - refresh functions from .func
        remf - remove a function
        seef - output the function body
        newv - create a variable
        remv - remove the variable
        lstv - list all variables
        renv - rename a variable
        setv - set variable's value with it's data type
        outv - output a single variable ($ before variable name)
        incv - increment a single variable as index ($ before variable name)
        decv - decrement a single variable as index ($ before variable name)
        fndk - find node names with a certain keyword (it is sensitive to capitalisations)
        fnds - find node names with a similar writing (it is sensitive to capitalisations)
        ersl - erase unsavable entries of functions and variables
        file - extract commands from file and execute them
        addv - add between variables ($ before variable name)
        subv - subtract between variables ($ before variable name)
        mulv - multiply between variables ($ before variable name)
        divv - divide between variables ($ before variable name)
        exit - exit

Also exists custom arguments:
        [--version | -v]  - get a version
        [-d]        - enter to debug mode (cache wouldn't be erased)
        [-h | --help]   - shows help for navigating the program
        [--argument | -a] - enter to argument mode (you can write all commands in arguments separated by space (for names as indexes it recognise automatically))
        [-ssa]      - you can type your input using a single string argument (f.e. -ssa "newp A newp B ...")

In argument mode:
        [-i] - alias to "load"
        [-o] - alias to "save"
        [-temp [i | o]] - alias to "tmpi" or "tmpo"

Also for functions:
        Functions can be with ! at the beginning to signature that they would not make an entry to .func
        These functions are not savable
Also for variables:
        [$(existing variable's name)] - insert variable's value right in to fields
        When you set the variable, you can set from graph values with $_
        The variable can not be named with _ at the beginning
        $_:size - size of the graph
        $_:origin - origin from where the operation can be passed
        definition: (your index or your variable or the variable name with $) (abbr. IN) - indexing based on node index
        definition: (your index or your variable) (abbr. IE) - indexing based on edge index in a node
        $_[IN] - index to the node
        $_[IN][IN] - boolean is the edge between these 2 nodes exists
        $_[IN][IN]:[to | from | cost] - the property to the edge
        $_[IN]:edges - amount of edges in the node
        $_[IN]:edges[IE]:[to | from | cost] - the property to the edge in indexed edges in the node
```

Installing
===============
In realese tab exists 8 binaries for x86_32, x86_64 and 2 with aarch64/32
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
If you're wanting to compile using MinGW or GCC, then you can just launch `build.sh` after `chmod a+x build.sh`.
If you're wanting to compile using MSVC, then here's a .sln project

It requires:
 -  c++17

That's it
===============
