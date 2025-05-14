[![INFORMS Journal on Computing Logo](https://INFORMSJoC.github.io/logos/INFORMS_Journal_on_Computing_Header.jpg)](https://pubsonline.informs.org/journal/ijoc)

# A Hybrid Genetic Algorithm with Multi-population for Capacitated Location Routing

This archive is distributed in association with the [INFORMS Journal on
Computing](https://pubsonline.informs.org/journal/ijoc) under the [MIT License](LICENSE).

The software and data in this repository are a snapshot of the software and data
that were used in the research reported in the paper _A Hybrid Genetic Algorithm with Multi-population for Capacitated Location Routing_ by P.F. He, J.K. Hao, and Q.H. Wu. 

## Cite

To cite the contents of this repository, please cite both the paper and this repo, using their respective DOIs.

https://doi.org/10.1287/ijoc.2023.0416

https://doi.org/10.1287/ijoc.2023.0416.cd


Below is the BibTex for citing this snapshot of the repository.

```
@misc{Hybrid2025,
  author =        {Pengfei He, Jin-Kao Hao and Qinghua Wu},
  publisher =     {INFORMS Journal on Computing},
  title =         {A Hybrid Genetic Algorithm with Multi-population for Capacitated Location Routing},
  year =          {2025},
  doi =           {10.1287/ijoc.2023.0416.cd},
  url =           {https://github.com/INFORMSJoC/2023.0416},
  note =          {Available for download at https://github.com/INFORMSJoC/2023.0416},
} 
```

## Running the programs

To generate the executable codes (ilrp) of HGAMPP algorithm respectively for the CLRP, one can run 'make' command under code folder in linux system.

 ### The CLRP
_Usage:_ 

./code/ilrp instance depot_config outputFile bestValue isOptimal timeLimit numOfRuns randomSeed

- 'instance' is the input benchamrk instance
- 'depot_config' denotes the input depot configuration associated with the input instance, and this parameter can be removed
- 'outputFile' is the name of output file 
- 'bestValue' is the best result found from literature.
- 'isOptimal' is used to indicate whether the bestValue is the optimal result.
- 'timeLimit' is the time limit of each run
- 'numOfRuns' is the maximum runs of each run
- 'randomSeed' is the random seed 


## Materials
This repository includes the following materials:

--Benchmark instances used in our paper (See the file named instances). One notices that the file also includes a set of depot configurations which are used for HGAMP. Of course, you can also adjust the code in commandline.h to overlook it. 
--Source codes of proposed HGAMP (See the source codes directory for the details.)   
--Detailed computational results and parameter analysis (See the detailed results directory for the details.)   
--Best solutions found in the experiments (See the certificate file for the details.)  
Note: The contents and formats of the files are demonstrated in the ReadMe file of corresponding subdirectory.  
  
