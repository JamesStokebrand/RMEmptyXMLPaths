RMEmptyXMLPaths
===============

Takes an SVG file from Inkscape, scans the file for empty XML paths, and removes the paths from the output file.

Usage:
argv[0] <input_SVG_file>.svg 

Output
<input_SVG_file>_out.svg

Opens the input SVG file, reads the file char by char and parses out the <path /> XML strings.
Searches inside each string for the empty path marker "0,0/"" and does not write this empty path to the output SVG file.
Assumes the input file is a correctly formatted SVG file produced by the Inkscape vector graphic program.  

