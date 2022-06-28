## ARM11 C Project - Group 13

## Source Code [Present in the src directory]

- emulate.c contains the functionality for the **Emulator**.
    - /emulate_tools contains helper files for emulate.

- assemble.c contains the functionality for the **Assembler**.
    - /assemble_tools contains helper files for assemble.

- defns.h and instruction.h contains global structs that are utilized by the Emulator and the Assembler

- Makefile: You can use ` $make ` to generate `./emulate` and `./assemble` executables.

## Documentation [Present in the doc directory]

- Checkpoint.tex is the checkpoint report (emulate).

- Report.tex is the final report (assemble + extension).

- Makefile: You can use ` $make ` to generate report PDF's.

## Extension [Present in the extension directory]

- trans_Analyser.c contains the functionality for the Extension on **Transaction Analyser and Visualiser** (Serialisability and Recoverability).

- plot.py contains a script to plot the conflicts discovered by trans_Analyser.c in a pdf file

- run.sh contains the bash script to make trans_Analyser and plot the conflicts and then clean out temporary out files in the directory upon execution

- Requirement - NetworkX package for plot.py (Package for the creation, manipulation, and study of complex networks.)

- Install the latest version of NetworkX:
 ```
$ pip install networkx[default]
 ```

## Running Extension [in extension directory]

- Use `./run.sh <input filename>` to see end-to-end solution in generated **graph.pdf** file

- Note: 
    - ignore GDK critical errors when runnning over ssh / use ssh with -X flag to avoid this
    - See input syntax from sample file provided as follows:   
        - Line 1        : number of individual histories (n)
        - Line 2-n+1    : different histories
        - Final Line    : Target history (to be analysed)
