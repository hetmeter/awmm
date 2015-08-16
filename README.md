# awmm
BSc thesis - Analysis for Weak Memory Models

Installation instructions:

Download and copy the entirety of the /install/ folder. Run run_abstraction.bat INPUT_PATH BUFFER_SIZE_LIMIT CUBE_SIZE_LIMIT

The file specified as the INPUT_PATH must have the extension .sal and there must be a file of the same name with the extension .predicate.sal in the same folder. E.g.:
abp.sal
abp.predicate.sal

BUFFER_SIZE_LIMIT defines K as described in the thesis.

CUBE_SIZE_LIMIT is a positive integer that defines an inclusive upper bound on the sizes of all cubes to be considered in the predicate abstraction phase.