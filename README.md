# maps_parser

The PMapParser class parses the output of the Linux pmap -x [PID] command and performs some basic calculations.
The std::system function is used to streamline a number of terminal commands into its operation.
The PMapParser is designed to parse the pmap output for all PIDs of a running program and perform some basic calculations.
It can provide data for a summation of PIDs, a single PID, or a single mapping within a particular PID.
Permissions are counted and recorded, as well as total mapping size, resident set size (RSS), and number of dirty bytes.
